# STM32 Bare-Metal Storage & Logging System

Este repositório contém uma biblioteca e arquitetura de sistema de arquivos *bare-metal* (C/C++) otimizada para microcontroladores STM32. O sistema foi inicialmente projetado para gerenciar o log de dados de alta frequência na aviônica de foguetes experimentais do **Projeto Júpiter (Poli-USP)**, lidando com armazenamento concorrente em memória Flash SPI (W25Qxx) e cartão SD via SDIO.

## 🚀 Principais Funcionalidades

*   **Abstração Orientada a Objetos (C++):** Envelopamento seguro das bibliotecas `FatFs` (SD Card) e `LittleFS` (Flash SPI) através das classes `SDCard` e `FlashStorage`.
*   **Gravação Assíncrona (Non-Blocking):** Implementação de um buffer circular (`FifoBuffer`) na classe `FlashStorage`. A máquina de estados (`update()`) gerencia a escrita assíncrona, evitando que a CPU perca ciclos bloqueada no barramento SPI — algo crítico durante o voo de um foguete.
*   **Configuração Dinâmica:** A classe `ConfigLoader` permite o carregamento de configurações de missão (filtros, triggers da FSM, frequências de rádio) a partir de um arquivo texto (SD ou Flash), eliminando a necessidade de recompilar o firmware em campo.
*   **Rotinas de Extração (Dump) Pós-Voo:** O `Storage_Logger` automatiza a extração de pacotes binários gravados em voo na Flash e os converte em múltiplos arquivos CSV formatados diretamente no SD Card, prontos para análise em planilhas ou Python.
*   **Adaptação Dinâmica da Flash:** O port do LittleFS lê o JEDEC ID do chip Winbond dinamicamente para calcular a contagem de blocos e capacidade de armazenamento.

## 📂 Estrutura do Repositório

O projeto é dividido em camadas de abstração para facilitar o reuso das bibliotecas em diferentes contextos:

```text
Storage/
│
├── Drivers/                  # <- Drivers de baixo nível e abstração do File System
│   ├── FlashStorage.cpp/h    # Interface C++ e FIFO assíncrona para a Flash
│   ├── SDCard.cpp/hh         # Interface C++ para FatFs e comandos de diretório
│   ├── lfs_port_w25qxx.c/h   # Port em C puro do LittleFS para memórias Winbond W25Qxx
│   └── FifoBuffer.hh         # Template de Buffer Circular genérico
│
└── Application/              # <- Regra de negócio, Loggers e Configurações
    ├── Storage_Logger.cpp/h  # Máquina de estados para gravação de pacotes e dump CSV
    ├── DataFormatter.cpp/h   # Cabeçalhos e serialização das structs em CSV
    └── ConfigLoader.cpp/h    # Parser de arquivos texto ('=') para carregar configurações
```

## ⚙️ Requisitos do STM32CubeIDE

Para integrar esta biblioteca corretamente usando o gerador de código (STM32CubeMX):

1.  **SDIO e DMA:** Habilite os canais de DMA de **RX** e **TX** para a interface SDIO. No middleware do `FATFS`, defina o *Use dma template* como ativo. Ative as interrupções globais de SDIO na NVIC.
2.  **Aumento de Memória:** Vá até a aba *Project Manager* > *Code Generator* e aumente significativamente o *Minimum Heap Size* (ex: `0x2000`) e *Minimum Stack Size* (ex: `0x2000`). O FatFs exige mais espaço nativo na Stack, e não dimensionar isso pode causar *HardFaults* silenciosos.
3.  **Timeout do SD:** Em certos cartões SD, o timeout padrão do STM32 pode ser muito curto, gerando erros ao criar arquivos. Recomenda-se alterar o valor de `#define SD_TIMEOUT` dentro do arquivo gerado `sd_diskio.c`.
4.  **SPI:** O módulo da Flash SPI (`W25Qxx`) espera ser inicializado com a declaração do *handle* correto (ex: `&hspi1`) e as portas de *Chip Select* (CS).

## 💻 Exemplo Básico de Uso (Application Layer)

Este exemplo demonstra como inicializar o Logger, salvar configurações, e rodar a máquina de estados assíncrona no loop principal (`main.cpp`).

```cpp
#include "Storage_Logger.h"
#include "ConfigLoader.h"

// Instanciação Global (Substitua SPI1, GPIOA e GPIO_PIN_4 pelas suas portas)
Storage_Logger logger(&hspi1, {GPIOA, GPIO_PIN_4}, {nullptr, 0});
ConfigLoader config;

// Struct de exemplo simulando um pacote inercial
struct ImuPacket {
    uint32_t timestamp;
    float accelZ;
};

void setup() {
    // 1. Inicializa o SD e a Flash, e gera arquivos com um prefixo
    logger.init(false, "FLIGHT_LOG", "SYS_STATUS");

    // 2. Carrega configurações de um arquivo 'config.txt' no SD Card
    if (config.loadFromSD(logger.sdManager, "config.txt")) {
        config.printConfig();
    }
    
    // 3. Abre o arquivo binário na Flash para gravação assíncrona
    logger.openFlash();
}

void loop() {
    ImuPacket data = {HAL_GetTick(), 9.81f};

    // 4. Salva o pacote na FIFO (Operação não bloqueante)
    logger.writeData(1, &data);

    // 5. Máquina de estados: Descarrega a FIFO da RAM para a Flash física
    logger.processFlashQueue();
}

int main(void) {
    HAL_Init();
    // (Inicialização dos periféricos gerada pelo MX)
    
    setup();

    while (1) {
        loop();
    }
}
```

## 🔄 Extração (Dump Pós-Voo)

No módulo `Storage_Logger`, o método `dumpFlashToSD()` percorre o log binário salvo na memória Flash SPI, identifica os *IDs* dos pacotes estruturados, e converte-os (através dos callbacks do `DataFormatter`) em planilhas `.csv` legíveis salvas diretamente no cartão SD.

---
**Autor:** Giovanni Antônio Silva de Oliveira  
**Projeto Júpiter - Escola Politécnica da USP**
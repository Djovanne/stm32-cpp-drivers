# stm32-cpp-drivers
Drivers e bibliotecas, majoritariamente em C++, para o ambiente STM32CubeIDE e STM32 HAL
# stm32-cpp-drivers

Drivers e bibliotecas, majoritariamente em C++, para o ambiente STM32CubeIDE e STM32 HAL.

Este repositório é uma coleção de abstrações orientadas a objetos desenvolvidas para microcontroladores STM32. O objetivo principal é trazer as vantagens do C++ (encapsulamento, tipagem forte, reuso de código) para sistemas embarcados *bare-metal*, mantendo a alta performance e o baixo *overhead* exigidos por aplicações críticas, sem a necessidade estrita de um RTOS (inicialmente).

Grande parte destas bibliotecas foi concebida e testada nos sistemas de aviônica e telemetria de foguetes experimentais do **Projeto Júpiter** (Escola Politécnica da USP).

## 📂 Estrutura do Repositório

O repositório está dividido em duas áreas principais: as bibliotecas genéricas (Drivers) e as implementações práticas (Examples).

```text
stm32-cpp-drivers/
│
├── Drivers/                  # <- Bibliotecas, Wrappers e Módulos reutilizáveis
│   ├── Storage/              # Sistema de arquivos assíncrono (Flash SPI W25Qxx + SD Card)
│   ├── USB_Serial/           # Roteamento nativo de printf() via USB CDC (Virtual COM Port)
│   └── ...                   # (Outros drivers futuros)
│
└── Examples/                 # <- Projetos teste e demonstrações de uso
    ├── srad_logger_test/     # Exemplo de Logger de voo configurável gravando em Flash/SD
    ├── usb_terminal_test/    # Exemplo de loopback e telemetria via USB nativa
    └── ...
```

## 🧩 Módulos Disponíveis

### 1. Storage & Logging (Flash + SD)
Uma arquitetura de sistema de arquivos utilizando `FatFs` e `LittleFS`. Implementa um buffer circular (FIFO) que permite que a CPU mande pacotes de dados para a memória Flash de forma assíncrona. Inclui também classes para carregar configurações dinamicamente via arquivos texto (`.ini`/`.txt`).
👉 [Ver documentação do módulo de Storage](Drivers/Storage/README.md)

### 2. USB Serial (CDC)
Um *wrapper* que sobrescreve a chamada `_write` do sistema, permitindo usar a função padrão `printf()` da linguagem C diretamente pela porta USB nativa do microcontrolador. Inclui proteção via sinal DTR (Data Terminal Ready) para evitar que o microcontrolador trave esperando conexão caso o cabo USB seja removido.
👉 [Ver documentação do módulo USB](Drivers/USB_Serial/README.md)

### 3. Modules/Shared
👉 Uma pasta que consta projetos mais antigos e algumas modificacoes de perifericos da ST, nao está muito documentada, mas a implementação é simples, então creio que não seja problemático usa-las por agora

## ⚙️ Como usar no STM32CubeIDE

Para utilizar qualquer um destes drivers no seu projeto gerado pelo STM32CubeMX:

1. **Converta o projeto para C++:** No STM32CubeIDE, clique com o botão direito no projeto no *Project Explorer* -> `Convert to C++`. Renomeie o seu `main.c` para `main.cpp`.
2. **Adicione os arquivos:** Copie a pasta do driver desejado para dentro do seu projeto (ex: na pasta `Core/Src/` ou crie uma pasta `Middlewares/Custom/`).
3. **Include Paths:** Não se esqueça de adicionar os caminhos dos novos arquivos `.h`/`.hh` nas configurações de *Include Paths* do compilador (Project Properties -> C/C++ Build -> Settings -> MCU G++ Compiler -> Include paths).
4. **Dependências do HAL:** As bibliotecas dependem da inicialização correta dos periféricos (SPI, SDIO, USB) gerada pelo STM32CubeMX. Verifique o `README.md` de cada módulo para configurações específicas (como ativação de DMA ou aumento de Heap/Stack).

## Configurando projeto

Para poder usar a IDE sem ficar misturando codigo do MX com o seu, que é C++, vc pode fazer a seguinte estrutura usando Wrapper:

**1. Padrão Wrapper (`extern "C"`):**
Crie um cabeçalho `App_main.hh` com a diretiva `extern "C"` para evitar o *Name Mangling* do C++:
```cpp
#ifndef INC_APP_MAIN_HH_
#define INC_APP_MAIN_HH_

#ifdef __cplusplus
extern "C" {
#endif

void app_main(); // Ponto de entrada da aplicação C++

#ifdef __cplusplus
}
#endif
#endif
```
**2. Integração no `main.c`:**
No `main.c` gerado pelo CubeMX, inclua o cabeçalho e chame a função logo antes do loop infinito:
```c
/* USER CODE BEGIN Includes */
#include "App_main.hh"
/* USER CODE END Includes */

/* USER CODE BEGIN 2 */
app_main(); // Transfere o controle para a camada C++
/* USER CODE END 2 */
```
**3. Codando:**
Por fim, crie o arquivo e funcao app_main(), e code normalmente lá em C++ (use ctrl+space para completar o include e ver se a IDE identifica que a pasta está no path)
```c
#include "app_main.h"
#include "DigitalOut.h"
#include "Timer.h"

void app_main()
{
	HAL_delay(100);
	
	DigitalOut led(GPIOA,GPIO_PIN_1);
	Timer ledTimer(1000);
	
	led = 0;
	printf("Código iniciado!\n\r");
	
	while(true)
	{
		if(ledTimer.checkAndReset())
		{
			led.toggle();
			printf("Piscando...\n\r");
		}
	}
	return;
}
```
## 📄 Licença

Este projeto é distribuído sob a licença MIT. Veja o arquivo `LICENSE` para mais detalhes.

---
**Desenvolvido por:** Giovanni Antônio Silva de Oliveira  
**Contexto:** Projeto Júpiter - Escola Politécnica da Universidade de São Paulo (USP)

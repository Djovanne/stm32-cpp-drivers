## 🔌 Integração USB Serial (CDC Virtual COM Port)

Este projeto inclui uma interface de roteamento para a USB nativa do STM32, utilizando a classe CDC (Communication Device Class). Isso elimina a necessidade de conversores USB-Serial, permitindo ler os dados e logs ligando o cabo USB diretamente no microcontrolador.

### Destaques da Implementação USB
* **Redirecionamento Direto:** O arquivo `USBSerial.c` sobrescreve a chamada de sistema `_write`, roteando nativamente qualquer chamada de `printf()` para o barramento USB (`CDC_Transmit_FS`).
* **Detecção de Terminal Seguro (DTR):** Possui um mecanismo de proteção (`g_cdc_terminal_open`) que detecta quando o sinal DTR (Data Terminal Ready) é ativado pelo PC. Isso impede que o firmware trave (timeout) tentando enviar pacotes se não houver um terminal (como PuTTY, Foxglove, etc.) ativamente aberto e escutando a porta.
* **USB Composto:** A implementação é compatível e inspirada pela biblioteca de classes compostas [I-CUBE-USBD-Composite](https://github.com/alambe94/I-CUBE-USBD-Composite).

### ⚙️ Requisitos e Configuração (STM32CubeIDE)

1. **Configuração no CubeMX:**
   * Ative o periférico **USB** ou **USB_OTG_FS** (modo *Device Only*).
   * Em **Middleware**, ative o **USB_DEVICE** com a classe **Communication Device Class (Virtual Port Com)**.
   * *(Opcional)* Para mudar o nome que aparece no PC, altere a opção `Product String` dentro das configurações do *Configuration Descriptor*.

2. **Modificação Obrigatória no `usbd_cdc_if.c`:**
   Para o mecanismo de detecção do terminal funcionar, você deve fazer um "hook" manual no código gerado pelo CubeMX.
   Abra o arquivo `usbd_cdc_if.c`, inclua o cabeçalho e adicione a função `DTR_Control` dentro de `CDC_Control_FS`:

   ```c
   /* 1. Adicione os includes no topo de usbd_cdc_if.c */
   #include "USBSerial.h"

   /* 2. Modifique a função de controle */
   static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
   {
       switch (cmd)
       {
           // ... (outros cases gerados automaticamente) ...
           
           case CDC_SET_CONTROL_LINE_STATE:
               DTR_Control(pbuf); // <--- ADICIONE ESTA LINHA AQUI
               break;
               
           // ...
       }
       return USBD_OK;
   }
   ```

### 💻 Exemplo de Uso

Após configurado, o uso no código da aplicação é invisível. Basta usar funções padrão da biblioteca C:

```cpp
#include <stdio.h>

void loop() {
    // Isso será enviado pela USB automaticamente, sem travar o microcontrolador
    // caso o cabo seja desconectado no meio do voo/operação.
    printf("Altitude atual: %.2f m\n", altitude_atual);
    HAL_Delay(100);
}
```
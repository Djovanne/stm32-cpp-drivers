  * Para usar esse código de exemplo, o código de aplicação em sí está na pasta GNSS/App/app_gnss.c
  *
  * Essa última é gerada como exemplo de implementação da biblioteca (que ainda presisou de diversos ajustes) para
  * funcionar em bare-metal (sem RTOS ou similar)
  *
  * Para habilitar apropriadamente a bilbioteca:
  *
  * - vá para o MX, habilite as portas UART1 e UART2 (a última para debug apenas, a principal que estara o módulo será a 1)
  * junto com os pinos de WakeUp e Reset aonde preferir. A porta UART que reseber os dados deve estar com o DMA e a interrupção no RX habilitada
  *
  * - Agora vá para Software Packs -> Select Components e selecione a biblioteca X-CUBE-GNSS1, e faça as série de configurações abaixo:
  *
  * 	1º - Expanda a aba da biblioteca, aparecerá abas como "Exposed APIs", "Board Extension", "Board Part" e etc.
  * 	2° - Em "Device GNSS1_Aplication", selecione "SimOSGetPos"
  * 	3° - Em "Data Exchange", expanda "lib_gnss" e habilite "libGNSS", "libNMEA", e como interface coloque "Basic"
  * 	4° - Em "Board Part", selecione a opção "Bare-Metal" para fazer a aplicação sem RTOS
  * 	5° - Em "Board Extension", marque a opção GNSS1A1
  * 	6º - A princípio, nenhum marcador deve estar com um sinal amarelo, e tudo verde, caso esteja algo diferente de verde, clique 2 vezes e resolva o problema que ele mostra
  *
  * - Agora vá para a aba do MX, ao lado de "Clock Configuration", "Project Manager" e selecione "Advanced Setting" na aba a esquerda.
  *
  * - Na aba direita "Register CallBack", mude de "Disable" para "Enable" a aba da UART e USART (ou só da que vc fez)
  *
  * - Para informações do uso da biblioteca, há uma documentação dela junto com quando vc baixa, que pode ser acessada em:
  *
  * 		C:\Users\'Seu usuário'\STM32Cube\Repository\Packs\STMicroelectronics\X-CUBE-GNSS1\7.1.0\Documentation\X-CUBE-GNSS1.chm
  *
  ******************************************************************************
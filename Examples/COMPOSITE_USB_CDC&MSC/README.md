Testa a implementação de uma classe USB composta, no caso, a implementação em conjunto de uma porta serial para Windows(CDC_ACM) e de dois sistemas de armazenamento (SD e EEPROM), com a classes MSC, em uma mesma USB
Ela usa uma lib, com o arquivo para download na pasta Libs, que foi instalado no MX e então selecionado as classes para mesclagem

Para Usar essa configuração:

- Aumente o "Minimum Heap Size" e o "Minimum Stack Size" para no mínimo o dobro do tamanho

- Clock do USB em 48 MHz (obrigatóriamente Externo)

- Em NVIC, prioridade do SysTick Maior que a do USB (Habilite o USB como Device, e ative a interrupção global), isso é necessário apenas para o caso de desconecção do SD Card, que é tentando reconectar com ele

- Na pasta "Composite", mude as interfaces de "True" para "1U", que o compilador interpreta como diferente, e acaba não ativando as classes

- Na pasta "Libs", há o instalador e o guia original para a instalação

- Devese também habilitar as interrupções do NVIC da USB, e chamar o método "MX_USB_DEVICE_Init();", e ativar o "Generate peripheral initialization as a pair of '.c/.h' files per perifheral" no Project Manager -> Code Generator no MX.

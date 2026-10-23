Este teste baseia-se na implementação própria da "Mass Storage Class" USB das duas memórias existentes (um SD Card e a EEPROM M95 da ST), que faz com que, ao conecta-las na porta USB, sejam reconhecidas pelo PC como "dispositivos externos de armazenamento", e possibilita a leitura e escrita de ambas.

É interessante notar que toda a Implementação e configuração da USB se encontra na pasta "USB", na qual é especificado a forma de gracação das memórias (EEPROM na pasta memory, e o SD é mais direto graças as HAL), e as funcionalidades em sí são descritas no arquivo:

- USB/Class/MSC/Src/usbd_storage_if.c

é importante notar que, para o caso do SD Card, para ser possivel a inicialização dele após a retirada, é feita a reinicialização do SD_init toda vez que a USB pergunta se ainda está vivo, e o SD não responde, mas como o SD_Init usa um temporizador interno baseado em interrupção, deixe o USB com uma prioridade menor no NVIC para que o controlador não trave nesse delay, ou use um externo. Por simplicidade, foi diminuito a prioridade da USB.
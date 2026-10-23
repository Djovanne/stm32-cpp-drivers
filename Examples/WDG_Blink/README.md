O teste configura um pisca-pisca usando WatchDog com base no reinicio do controlador, que pode ser encontrado no System Core. 

- Para o calculo do tempo de delay no MX do IWDG (que tem um tempo máximo para ele ser resetado), podemos seguir a formula:

    RL = ((t_IWDG(ms)*LSI)/(4*(2^(log2(preescaler)-2))*1000))-1

Onde:
- - RL é o valor do "down-counter reload"
- - LSI é o valor da frequência do clock interno de baixa velocidade
- - preescaler é o fator de divisão do clock do IWDG
- - t_IWDG(ms) é o valor do tempo para o timer estourar e reiniciar o controlador

- Para o calculo do tempo de delay no MX do WWDG (Que pode tem que ser reiniciado em uma janela em específico, com um máximo e um mínimo), podemos seguir a formula:

    Cont = (TempoMax(ms)*Clock)/(4096*2^((log2(preescaler)-2))*1000)+64
    Win  = Cont - (TempoMin(ms)*Clock)/(4096*2^((log2(preescaler)-2))*1000)

Onde:
- - TempoMax(ms) e TempoMin(ms) é o tempo da janela, em milisegundos
- - Clock é a frequencia do barramento (no caso APB1)
- - preescaler é o fator de divisão do clock do WWDG
- - Win é o valor da janela do sistema (parâmetro de configuração)
- - Cont é o valor do free runing downCounter.

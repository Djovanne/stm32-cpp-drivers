Esse teste apenas faz a configuração da data e horário do dia, e ativa um alarme para estourar a meia noite no dia.

- A configuração é feita na aba "Timers -> RTC", aonde ativamos o "Activate Clock Source" e o "Activate Calendar" (para setar a data)

- Ativamos o alarme interno A, e ativamos o LSE da BlackPill em "System Core" -> "RCC" -> "Low Speed Clock(LSE)" -> "Crystal/Ceramic Resonator".

- Agora na aba "Clock Configuration", no topo no mux "RTC Clock Mux" selecionamos a fonte LSE para o registrador "To RTC (KHz)", que será o clock de funcionamento do relógio.

- Se quiser checar se o clock do RTC está correto, você pode em "RRC" na Pinout & Configuration ativar o Master Clock Output 1, quehabilitará um pino de saída de uma das fontes de clock do sistema, que pode ser selecionada no mux "MC01 source Mux"
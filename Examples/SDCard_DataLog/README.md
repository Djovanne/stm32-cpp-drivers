Para Configurar corretamente e poder usar o Middleware FATFS para usar um SD Card:

- Selecione, no MX, na aba "Conenctions" o protocolo "SDIO" ou qualquer outro do SD Card, habilite ele, a interrupção e o DMA RX e TX.

- Fabilite o Middleware FATFS, selecione "SD Card", em "Plarform Settings" coloque o pino de detecção do SD Card, e em "Advanced Settings" habilite "Use dma template".

- Em project Manager, na aba project, aumente o "minimum Heap Size " e "Minimum Stack Size" para umas 5x (0x1000 e 0x2000), pois os buffers internos da lib são pesados e grandes, então precisam de bastante espaço

# Campo Minado

## Introdução:

  
Campo minado é um jogo em que o jogador recebe um tabuleiro no qual um certo número de casas possui minas ocultas. Para vencer, o jogador deve selecionar todas as casas vazias, evitando as minas.

## Roteiro:  

O tabuleiro é uma matriz 5x14, que inicia com o conteúdo de todas as casas oculto, como visto na **Figura 1**. Assim que uma casa é selecionada, ela e seus vizinhos na matriz são revelados. Uma casa pode estar vazia ou conter uma mina. Casas vazias apresentam o valor total de minas adjacentes, ou nada, caso não tenham minas vizinhas.

<p align="center">
  <img src="https://lh6.googleusercontent.com/KbEe98pgpzxm7q4e9VcQOEofyWBAaHUlcj2RhR4-m04PyTyIWHOA9puv0zDMjeKwInRIX1IU-9gOdVK81d-xNBTXTny6y28bnryemjrImoKlRvcNOH4A_1uMyCLAtAFF3oH5MPz37HAhtLXGdg" />
</p>
<h4 align="center">
  Figura 1. Tabuleiro com todas as casas ocultas 
</h4>

Uma casa revela apenas a si mesma caso tenha alguma mina adjacente. Existem 14 minas no tabuleiro, e caso qualquer uma delas seja selecionada, o jogador perde o jogo. O jogador pode marcar casas onde acredita que existem minas com bandeiras para facilitar o jogo, como visto na **Figura 2**. Quando o jogador selecionar todas as casas do tabuleiro, exceto aquelas onde há minas, ele vence o jogo. Quando o jogo termina, o jogador pode reiniciar o jogo com um tabuleiro novo.

<p align="center">
  <img src="https://lh5.googleusercontent.com/YYUNc7a3Zyjsp2PkiYwr9oKhANGXT3BjsAiiDPv0pUN3DOSiZzZJ6VNPtvtt2hBacH--T7cb5FGjXnm3s1agOqbaCZqIhgSWBmLeQoq_-xLLOs_DSN3hV7vZbPOwz7XXkyPe1HgCuDzYVRuYfg" />
</p>
<h4 align="center">
  Figura 2. Tabuleiro parcialmente explorado 
</h4> 

## Fim de jogo:

 - ### Sucesso: todas as casas vazias foram selecionadas;

<p align="center">
  <img src="https://lh3.googleusercontent.com/hT1hObA7wl0n-DFspSWY9oqZcqxzFrFy-wbX45kgU8gtWPmFKnkYsOJvoYdukaPccwL5GdG_CSVp0S0cuDNf3TVIpi8lfbQEpzq89xvTZ4bcyC7DGBqev6Dap7am6csWeNbA_9UlmvSqTgGJ1A" />
</p>
<h4 align="center">
  Figura 3. Mensagem de vitória 
</h4>

O tabuleiro é revelado e uma mensagem de vitória aparece, como visto na **Figura 3**. O jogador deve então clicar no botão FLAG para reiniciar o jogo.

-   ### Falha: uma mina foi selecionada;

<p align="center">
  <img src="https://lh5.googleusercontent.com/RBUrfteJYom-YUTxq5mAH-nZEKsihsBRaMTrYpNQdtp2Hex8W2Zn6rRvUf-pYX44Q12I2JNTCZTVKLJSjKECg26K6JnEZkSPkYoJJCGJOV1lDq8-gAardsIcmct-zx-SXWq2xdHI2_gL3rYbeQ" />
</p>
<h4 align="center"> 
  Figura 4. Mensagem de derrota
</h4>

O tabuleiro é revelado e uma mensagem de derrota aparece, como visto na **Figura 4**. O jogador deve então clicar no botão FLAG para reiniciar o jogo.

## Recursos:

Para implementar esse jogo, foi usado o processador do AVR. Foram utilizados seis botões, sendo quatro direcionais e dois de seleção. Os direcionais alternam a casa a ser selecionada. O botão CHECK é responsável por iniciar o jogo saindo da tela inicial, e por selecionar as casas desejadas, revelando elas durante o jogo. O botão FLAG é responsável por reiniciar o jogo quando ele acabou, e marcar uma casa do tabuleiro com uma bandeira.

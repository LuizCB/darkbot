#define	I_SPEAK "Falo portugu�s e um pouco de ingl�s."
#define	L001(a,b) S("PRIVMSG %s :N�o foi ainda definida a senha, para o fazer: \2/msg %s PASS \
<senha_anterior> <senha_nova>\2 (j� que esta � a primeira vez que a senha � configurada, usar '0' como senha anterior)\n",a,b)
#define	L002(a,b,c) S("NOTICE %s :Removido ban permanente (permban) #%d, %s.\n",a,b,c)
#define	L003(a,b) S("NOTICE %s :N�o foi poss�vel abrir %s :(\n",a,b)
#define L004(a,b,c) S("PRIVMSG %s :%s, H� %d SEENs na minha base de dados.\n",a,b,c)
#define	L005(a,b) S("PRIVMSG %s :Sabes melhor que eu acerca disso, %s.\n",a,b)
#define	L006(a,b,c,d) S("PRIVMSG %s :%s, N�o vi %s %s\n",a,b,c,d)
#define	L007(a,b) S("NOTICE %s :T�pico autom�tico (Autotopic) em %s estava desligado\n",a,b)
#define	L008(a,b) S("NOTICE %s :T�pico autom�tico (Autotopic) em %s foi desligado\n",a,b);
#define	L009(a,b,c) S("NOTICE %s :T�pico autom�tico (Autotopic) em %s LIGADO: %s\n",a,b,c)
#define	L010(a,b,c) S("NOTICE %s :T�pico autom�tico (Autotopic) em %s ACTUALIZADO: %s\n",a,b,c)
#define	L011(a,b,c) S("PRIVMSG %s :%s, Processado! H� agora %d frases para execu��o casual (randomstuffs).\n",a,b,c)
#define	L012(a,b) S("NOTICE %s :Senha de %s actualizada.\n",a,b)
#define	L013(a) S("NOTICE %s :Senha incorrecta!\n",a)
#define	L014(a) S("NOTICE %s :Nada encontrado!\n",a)
#define	L015(a,b,c,d) S("NOTICE %s :Usu�rio removido: %s [%d:%d]\n",a,b,c,d)
#define	L016(a,b) S("PRIVMSG %s :%s: Erro de divis�o por zero!\n",a,b)
#define	L017(a,b) S("PRIVMSG %s :%s: Opera��o Incorrecta! (Usar entre os n�meros +, -, *, /, para somar, subtrair, multiplicar e dividir respectivamente)!\n",a,b)
#define	L018(a,b,c,d,e) S("KICK %s %s :%s (Adicionado usu�rio a ser ignorado #%d: %s)\n",a,b,c,d,e)
#define	L019(a,b,c) S("PRIVMSG %s :Adicionado usu�rio a ser ignorado #%d: %s!\n",a,b,c);
#define L020(a,b,c,d) S("NOTICE %s :Actualizado #%d: \37[\37%s\37]\37 info\2:\2 %s\n",a,b,c,d)
#define	L021(a,b,c,d) S("PRIVMSG %s :%s \37%s\37.. %d itens verificados.\n",a,b,c,d)
#define	L022(a,b,c) S("PRIVMSG %s :Encontrados mais que \37%d\37 itens, por favor tentar ser-se mais espec�fico\2:\2 \
%s\n",a,b,c);
#define	L023(a,b,c) S("PRIVMSG %s :Encontrei \37um\37 item, %s\2:\2 %s\n",a,b,c)
#define	L024(a,b,c,d) S("PRIVMSG %s :Encontrados \37%d\37 itens, %s\2:\2 %s\n",a,b,c,d)
#define	L025(a,b) S("PRIVMSG %s :Encontrados %ld temas repetidos. Foram removidos.\n",a,b)
#define	L026(a,b,c,d,e,f,g,h,i,j,k,l) S("PRIVMSG %s :An�lise a %s. Tenho %ld temas na minha Base de Dados. Tempo desde que me liguei: \
%d dias%s, %02d:%02d, Foram feitas %ld perguntas, adicionados %ld temas e removidos %ld. \
Tempo de processamento: %1.4lf segundo%s\n",a,b,c,d,e,f,g,h,i,j,k,l)
#define	L027(a,b,c,d,e,f,g,h,i,j,k,l) S("PRIVMSG %s :An�lise a %s. Tenho %ld temas na minha Base de Dados. Tempo desde que me liguei: \
%d horas%s, %d minuto%s, Foram feitas %ld perguntas, adicionados %ld temas e removidos %ld. \
Tempo de processamento: %1.4lf segundo%s\n",a,b,c,d,e,f,g,h,i,j,k,l)
#define	L028(a,b,c,d,e,f,g,h,i,j,k,l) S("PRIVMSG %s :An�lise a %s. Tenho %ld temas na minha Base de Dados. Tempo desde que me liguei: \
%d minuto%s, %d segundo%s, Foram feitas %ld perguntas, adicionados %ld temas e removidos %ld. \
Tempo de processamento: %1.4lf segundo%s\n",a,b,c,d,e,f,g,h,i,j,k,l)
#define	L029(a,b,c,d) S("PRIVMSG %s :%s: Removido o t�pico #%ld, \2%s\2.\n",a,b,c,d)
#define	L030(a,b,c) S("PRIVMSG %s :%s\2:\2 N�o consegui encontrar o t�pico %s. Remo��o abortada.\n",a,b,c)
#define	L029n(a,b,c,d) S("NOTICE %s :%s: Removido o t�pico #%ld, \2%s\2.\n",a,b,c,d)
#define	L030n(a,b,c) S("NOTICE %s :%s\2:\2 N�o consegui encontrar o t�pico %s. Remo��o abortada.\n",a,b,c)
#define	L031(a,b) S("NOTICE %s :Sintaxe: \2/msg %s SENHA <senha_anterior> <senha_nova>\2\n",a,b)
#define	L032(a)	Snow("QUIT :K\2\2illed (%s (manuten��o!)\n",a)
#define	L033(a)	S("NOTICE %s :Sintaxe: UP <#canal> <senha>\n",a)
#define	L034(a)	S("NOTICE %s :Sintaxe: OP <#canal> [nicks] <senha>\n",a)
#define	L035(a)	S("NOTICE %s :Sintaxe: DEOP <#canal> [nicks] <senha>\n",a)
#define	L036(a)	S("NOTICE %s :Especificar um nick!\n",a)
#define	L037(a,b) S("NOTICE %s :Mudan�a para /nick %s\n",a,b)
#define	L038(a,b) S("NOTICE %s :Tentar /nick %s-idiota.\n",a,b)
#define	L039(a,b) S("PRIVMSG %s :Saindo %s\n",a,b)
#define	L040(a,b) S("PRIVMSG %s :Entrando %s\n",a,b)
#define	L041(a)	S("PRIVMSG %s :Especificar um nick/canal!\n",a)
#define	L042(a)	S("NOTICE %s :Especificar o u@h para remo��o!\n",a)
#define	L043(a) S("NOTICE %s :Ban n�o existente.\n",a)
#define	L044(a)	S("NOTICE %s :Especificar o u@h para remover!\n",a)
#define	L045(a) S("NOTICE %s :Faz isso a outro nick, n�o a mim\n",a)
#define	L046(a)	S("PRIVMSG %s :Processamento de c�pia da Base de Dados executada.\n",a)
#define	L047(a,b) S("NOTICE %s :Sintaxe: %cAUTOTOPIC <t�pico>  (escrever um \"0\" (zero) para desligar a fun��o)\n",a,b)
#define	L048(a)	S("NOTICE %s :Sintaxe: SETCHAN <#canal>\n",a)
#define	L049(a,b) S("NOTICE %s :Canal inicial: %s\n",a,b)
#define	L050(a)	S("NOTICE %s :Sintaxe: SETCHAR <novo caracter de comando>\n",a)
#define	L051(a,b) S("NOTICE %s :Novo caracter de comando � agora: %c\n",a,b)
#define	L052(a)	S("NOTICE %s :Sintaxe: SETUSER <novo userid> (necess�rio reiniciar)\n",a)
#define	L053(a,b) S("NOTICE %s :userid � agora: %s\n",a,b)
#define	L054(a)	S("NOTICE %s :Sintaxe: SETNICK <novo_nick>\n",a)
#define	L055(a)	S("NOTICE %s :Sintaxe: ADDUSER <#canal> <*user@*.host> <n�vel> <senha>. ex; ADDUSER #darkbot \
*jason@*.superlink.net 3 sua_senha ... usar #* se pretende dar-se acesso a todos os canais.\n",a)
#define	L056(a)	sprintf(temp, "N�o foi feito \2%cSETINFO\2 ainda!",a)
#define	L057(a,b,c)	S("NOTICE %s :Adicionado usu�rio: %s - n�vel %d\n",a,b,c)
#define	L058(a,b,c)	S("NOTICE %s :Sintaxe: %c%s <u@h> [raz�o]\n",a,b,c)
#define	L059(a,b,c,d) S("NOTICE %s :Adicionado em permban #%d, %s; raz�o: %s\n",a,b,c,d)
#define	L060(a)	S("NOTICE %s :Sintaxe: REPEAT <n�mero de vezes> <intervalo> <data>\n",a)
#define	L061(a)	S("NOTICE %s :Feito.\n",a)
#define	L062(a) S("QUIT :Reiniciando %s ...\n",a);
#define	L064(a,b) S("PRIVMSG %s :%s, o que � necess�rio adicionar?\n",a,b)
#define	L064n(a,b) S("NOTICE %s :%s, o que � necess�rio adicionar?\n",a,b)
#define	L065(a,b) S("PRIVMSG %s :Adicionar o qu�, %s?\n",a,b);
#define	L066(a,b,c)	S("PRIVMSG %s :%s: Que informa��o � para ser adicionada ao t�pico %s?\n",a,b,c)
#define	L067(a,b) S("PRIVMSG %s :Ok, %s :)\n",a,b)
#define	L065n(a,b) S("NOTICE %s :Adicionar o qu�, %s?\n",a,b);
#define	L066n(a,b,c) S("NOTICE %s :%s: Que informa��o � para ser adicionada ao t�pico %s?\n",a,b,c)
#define	L067n(a,b) S("NOTICE %s :Ok, %s :)\n",a,b)
#define	L068(a,b) S("PRIVMSG %s :Substituir o qu�, %s?\n",a,b)
#define	L069(a,b,c) S("PRIVMSG %s :%s: Que informa��o deve ser substitu�da %s?\n",a,b,c)
#define	L070(a,b,c) S("PRIVMSG %s :%s, %s substitu�do com sucesso.\n",a,b,c)
#define	L071(a,b,c,d) S("PRIVMSG %s :%s h� %d Ban permanente%s em mem�ria.\n",a,b,c,d)
#define	L073(a,b,c) S("PRIVMSG %s :%s, faltam %d segundos at� � pr�xima frase casual (randstuff)\n",a,b,c)
#define	L074(a,b,c)	S("PRIVMSG %s :%s: tinha %d caracteres.\n",a,b,c)
#define	L075(a,b,c,d) S("PRIVMSG %s :%s, h� neste momento %d iten%s no ponto.\n",a,b,c,d)
#define	L076(a,b) S("PRIVMSG %s :Vi %d entradas de usu�rios no canal at� ao momento.\n",a,b)
#define	L077(a,b,c,d,e)	S("PRIVMSG %s :H� %s %d server%s na minha lista de servers. Encontro-me neste momento no \
server #%d.\n",a,b,c,d,e)
#define	L078(a,b,c)	S("PRIVMSG %s :%s, o meu caracter de comando �: %c\n",a,b,c)
#define	L079(a,b,c)	S("PRIVMSG %s :%s??? Procuro o qu� %s?\n",a,b,c)
#define	L080(a,b) S("PRIVMSG %s :%s, � necess�rio especificar um endere�o!\n",a,b)
#define	L081(a,b) S("PRIVMSG %s :impossibilitado em verificar %s.\n",a,b)
#define	L083(a,b,c)	S("PRIVMSG %s :%s: O que digo a %s?\n",a,b,c)
#define	L084(a,b,c)	S("PRIVMSG %s :%s, Digo a %s o qu�?\n",a,b,c)
#define	L085(a,b) S("PRIVMSG %s :Digo a quem, %s?\n",a,b)
#define	L083n(a,b,c) S("NOTICE %s :%s: O que digo a %s?\n",a,b,c)
#define	L084n(a,b,c) S("NOTICE %s :%s, Digo a %s o qu�?\n",a,b,c)
#define	L085n(a,b) S("NOTICE %s :Digo a quem, %s?\n",a,b)
#define	L086(a,b) S("PRIVMSG %s :%s\2:\2 � uma pergunta?\n",a,b)
#define	L087(a,b,c,d,e) S("PRIVMSG %s :%s %s %s? Por favor repetir a pergunta  (escrever %cHELP para o sintaxe ta mesma \
hints)\n",a,b,c,d,e)
#define	L086n(a,b) S("NOTICE %s :%s\2:\2 � uma pergunta?\n",a,b)
#define	L087n(a,b,c,d,e) S("NOTICE %s :%s %s %s? Por favor repetir a pergunta  (escrever %cHELP para o sintaxe ta mesma \
hints)\n",a,b,c,d,e)
#define	L088(a) S("QUIT :%s est� lento!\n",a)
#define	L089(a)	S("PRIVMSG %s :\1ACTION acorda...\1\n",a)
#define	L090(a,b) S("PRIVMSG %s :estou a ser alvo de flood. Esvaziando itens do ponto. \
%d itens removidos.\n",a,b)
#define L091(a) S("NOTICE %s :Sintaxe: SETVHOST <Vhost novo> (requer que se reinicie)\n",a)
#define L092(a,b) S("NOTICE %s :Vhost est� agora configurado para: %s\n",a,b)

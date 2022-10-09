#include  <stdio.h>

int main (void){
        int porta;
        char ip[16];
	
	printf("Meu programa em C\n");
	printf("Digite o IP \n");
	scanf("%s",&ip);
	
	printf("Digite a porta \n");
	scanf("%i",&porta);
	
	printf("Varrendo Host: %s na porta %i \n",ip,porta);
	
	return 0;
}

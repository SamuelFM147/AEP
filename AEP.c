#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>
#include <time.h>

#define TAMANHO_MAX_USUARIO 50
#define TAMANHO_MAX_SENHA 50
#define ARQUIVO_DADOS "usuarios.txt"
#define CHAVE_CRIPTOGRAFIA 10 

typedef struct {
    char usuario[TAMANHO_MAX_USUARIO];
    char senha_criptografada[TAMANHO_MAX_SENHA * 2];
    char funcao[15];
} Usuario;

char gerar_caractere_aleatorio() {
    const char caracteres_extras[] = "!@#$%^&*()-_=+[]{}|;:,.<>?";
    int indice = rand() % (sizeof(caracteres_extras) - 1);
    return caracteres_extras[indice];
}

void criptografar_senha(char *senha, char *senha_criptografada) {
    int i, j = 0;
    srand(time(NULL));
    
    for (i = 0; i < strlen(senha); i++) {
        senha_criptografada[j++] = (senha[i] ^ CHAVE_CRIPTOGRAFIA) + CHAVE_CRIPTOGRAFIA;
        senha_criptografada[j++] = gerar_caractere_aleatorio();
    }
    senha_criptografada[j] = '\0';
}

void descriptografar_senha(char *senha_criptografada, char *senha) {
    int i, j = 0;
    
    for (i = 0; i < strlen(senha_criptografada); i += 2) {
        senha[j++] = (senha_criptografada[i] - CHAVE_CRIPTOGRAFIA) ^ CHAVE_CRIPTOGRAFIA;
    }
    senha[j] = '\0';
}

int verificar_requisitos_senha(char *senha) {
    if (strlen(senha) < 8) return 0;

    int tem_numero = 0, tem_caractere_especial = 0;
    int i;
	for (i = 0; senha[i] != '\0'; i++) {
        if (isdigit(senha[i])) tem_numero = 1;
        if (ispunct(senha[i])) tem_caractere_especial = 1;
    }
    return tem_numero && tem_caractere_especial;
}

void adicionar_usuario() {
    FILE *arquivo = fopen(ARQUIVO_DADOS, "a");
    if (!arquivo) {
        printf("Erro ao abrir o arquivo.\n");
        return;
    }

    Usuario usuario;
    char senha[TAMANHO_MAX_SENHA];

    printf("Digite o nome de usuário: ");
    scanf("%s", usuario.usuario);

    printf("Digite a senha (mínimo 8 caracteres, com número e caractere especial): ");
    scanf("%s", senha);

    if (!verificar_requisitos_senha(senha)) {
        printf("Senha não atende aos requisitos.\n");
        fclose(arquivo);
        return;
    }

    criptografar_senha(senha, usuario.senha_criptografada);

    printf("Digite o tipo de usuário ('admin' ou 'user'): ");
    scanf("%s", usuario.funcao);

    fprintf(arquivo, "%s %s %s\n", usuario.usuario, usuario.senha_criptografada, usuario.funcao);
    fclose(arquivo);

    printf("Usuário adicionado com sucesso!\n");
}

void listar_usuarios() {
    FILE *arquivo = fopen(ARQUIVO_DADOS, "r");
    if (!arquivo) {
        printf("Nenhum usuário cadastrado.\n");
        return;
    }

    Usuario usuario;
    printf("Lista de usuários:\n");
    while (fscanf(arquivo, "%s %s %s\n", usuario.usuario, usuario.senha_criptografada, usuario.funcao) != EOF) {
        printf("Usuário: %s, Função: %s\n", usuario.usuario, usuario.funcao);
    }

    fclose(arquivo);
}

void alterar_usuario() {
    char usuario_nome[TAMANHO_MAX_USUARIO];
    printf("Digite o nome de usuário que deseja alterar: ");
    scanf("%s", usuario_nome);

    FILE *arquivo = fopen(ARQUIVO_DADOS, "r");
    FILE *temp = fopen("temp.txt", "w");
    if (!arquivo || !temp) {
        printf("Erro ao abrir o arquivo.\n");
        return;
    }

    Usuario usuario;
    char senha_antiga[TAMANHO_MAX_SENHA], senha[TAMANHO_MAX_SENHA];
    int encontrado = 0;

    while (fscanf(arquivo, "%s %s %s\n", usuario.usuario, usuario.senha_criptografada, usuario.funcao) != EOF) {
        if (strcmp(usuario.usuario, usuario_nome) == 0) {
            encontrado = 1;
            char senha_descriptografada[TAMANHO_MAX_SENHA];
            descriptografar_senha(usuario.senha_criptografada, senha_descriptografada);

            printf("Digite a senha antiga: ");
            scanf("%s", senha_antiga);

            if (strcmp(senha_antiga, senha_descriptografada) != 0) {
                printf("Senha antiga incorreta.\n");
                fclose(arquivo);
                fclose(temp);
                remove("temp.txt");
                return;
            }

            printf("Digite a nova senha (mínimo 8 caracteres, com número e caractere especial): ");
            scanf("%s", senha);

            if (!verificar_requisitos_senha(senha)) {
                printf("Senha não atende aos requisitos.\n");
                fclose(arquivo);
                fclose(temp);
                remove("temp.txt");
                return;
            }

            criptografar_senha(senha, usuario.senha_criptografada);
        }
        fprintf(temp, "%s %s %s\n", usuario.usuario, usuario.senha_criptografada, usuario.funcao);
    }

    fclose(arquivo);
    fclose(temp);

    remove(ARQUIVO_DADOS);
    rename("temp.txt", ARQUIVO_DADOS);

    if (encontrado) {
        printf("Usuário alterado com sucesso!\n");
    } else {
        printf("Usuário não encontrado.\n");
    }
}

void excluir_usuario() {
    char usuario_nome[TAMANHO_MAX_USUARIO], senha[TAMANHO_MAX_SENHA];
    printf("Digite o nome de usuário que deseja excluir: ");
    scanf("%s", usuario_nome);

    printf("Digite a senha para confirmar: ");
    scanf("%s", senha);

    FILE *arquivo = fopen(ARQUIVO_DADOS, "r");
    FILE *temp = fopen("temp.txt", "w");
    if (!arquivo || !temp) {
        printf("Erro ao abrir o arquivo.\n");
        return;
    }

    Usuario usuario;
    int encontrado = 0;

    while (fscanf(arquivo, "%s %s %s\n", usuario.usuario, usuario.senha_criptografada, usuario.funcao) != EOF) {
        char senha_descriptografada[TAMANHO_MAX_SENHA];
        descriptografar_senha(usuario.senha_criptografada, senha_descriptografada);

        if (strcmp(usuario.usuario, usuario_nome) == 0 && strcmp(senha, senha_descriptografada) == 0) {
            encontrado = 1;
        } else {
            fprintf(temp, "%s %s %s\n", usuario.usuario, usuario.senha_criptografada, usuario.funcao);
        }
    }

    fclose(arquivo);
    fclose(temp);

    if (encontrado) {
        remove(ARQUIVO_DADOS);
        rename("temp.txt", ARQUIVO_DADOS);
        printf("Usuário excluído com sucesso!\n");
    } else {
        remove("temp.txt");
        printf("Usuário ou senha incorreta.\n");
    }
}

void limpar_tela() {
    system("cls");  
}

int main() {
    int opcao;
    
    setlocale(LC_ALL,"Portuguese");

    do {
        limpar_tela();  

        printf("\n--- Sistema de Gerenciamento de Usuários ---\n");
        printf("1. Incluir novo usuário\n");
        printf("2. Alterar usuário\n");
        printf("3. Excluir usuário\n");
        printf("4. Listar usuários\n");
        printf("0. Sair\n");
        printf("Escolha uma opção: ");
        scanf("%d", &opcao);

        limpar_tela();  

        switch (opcao) {
            case 1:
                adicionar_usuario();
                break;
            case 2:
                alterar_usuario();
                break;
            case 3:
                excluir_usuario();
                break;
            case 4:
                listar_usuarios();
                break;
            case 0:
                printf("Saindo...\n");
                break;
            default:
                printf("Opção inválida!\n");
        }
        
        printf("\nPressione Enter para continuar...");
        getchar(); getchar();  

    } while (opcao != 0);

    return 0;
}


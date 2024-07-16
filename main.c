#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#define SIZE 30
#define size 30
#define MAX_USERS 100
#define MAX_USERNAME 50
#define MAX_PASSWORD 50

//-----------------------definir cores-------------------

#define BLK "\e[0;30m"
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YEL "\e[0;33m"
#define BLU "\e[0;34m"
#define MAG "\e[0;35m"
#define CYN "\e[0;36m"
#define WHT "\e[0;37m"
#define textoMenu WHT

//Regular background
#define BLKB "\e[40m"
#define REDB "\e[41m"
#define GRNB "\e[42m"
#define YELB "\e[43m"
#define BLUB "\e[44m"
#define MAGB "\e[45m"
#define CYNB "\e[46m"
#define WHTB "\e[47m"


// Declara��o de ponteiros para arquivos
FILE *pont_arq;
FILE *pont_arq2;



// Defini��o da estrutura para armazenar informações do usuário
struct Usuario
{
    char Login[MAX_USERNAME];
    char Senha[MAX_PASSWORD];
    int Privilegio;
};

struct Data
{
    int dia;
    int mes;
    int ano;
};

typedef struct
{
    int id;
    char nome[size];
    int quantidade;
    int ativo;  //: quantidade em estoque
    // Outros campos do produto podem ser adicionados aqui
} Produto;

typedef struct
{
    int id_produto;
    float custo;
    int quantidade;  // Nova adi��o: quantidade comprada
    struct Data data;
} Compra;

int Confirma = 0;
float totalPorMes[12] = {0};

//=------------------------------Funçoes de Login--------------------------------


///Assinaturas de fun��es
void limparBuffer();
void clearScrean();
void pause();
void cadastro(struct Usuario novoUsuario, struct Usuario *usuarios, int *numUsuarios);
struct Usuario VerificarLogin(struct Usuario usuarios[], int numUsuarios);
void EditarUsuario(struct Usuario usuarios[], int numUsuarios);
void DeletarUsuario(struct Usuario usuarios[], int *numUsuarios);
int UsuarioExiste(struct Usuario usuarios[], int numUsuarios, const char *login);
void OpcoesDeUsuario(struct Usuario usuarios[], int *numUsuarios);
void menu(struct Usuario usuarios[], int numUsuarios, struct Usuario usuarioLogado);
void TratarErrosArquivo(FILE *pont_arq);
void CriptografarSenha(char *senha);

//--------------------------------novas adições-----------------------------------

void salvarProdutos(Produto produtos[], int numProdutos);
void carregarProdutos(Produto produtos[], int *numProdutos);
void salvarCompras(Compra compras[], int numCompras, Produto produtos[], int numProdutos);
void salvarComprasPorAno(Compra compras[], int numCompras, Produto produtos[], int numProdutos, float totalPorMes[]);
void salvarComprasNaDB(Compra compras[], int numCompras);
void carregarComprasDaDB(Compra compras[], int *numCompras);

void calcularTotalPorMes(Compra compras[], int numCompras, float totalPorMes[]);
int validarData(int dia, int mes, int ano);
int diasNoMes(int mes, int ano);
int lerData(int *dia, int *mes, int *ano);
void saidaEstoque(Produto produtos[], int numProdutos);
void registrarSaida(const char *nomeProduto, int quantidadeSaida);
//---------------------------------------------------------------------------
void cadastroProd(Produto produtos[], int numProdutos);
void checagemEstoque(Produto produtos[], int numProdutos, Compra compras[], int numCompras);
int cadastroEstoque(Compra compras[], int numCompras, Produto produtos[], int numProdutos);
void historicoCusto(Compra compras[], int numCompras);
void desativarProduto(Produto produtos[], int numProdutos);
void ativarProduto(Produto produtos[], int numProdutos);

int main(void)
{
    setlocale(LC_ALL, "Portuguese");
    system("color");



    struct Usuario usuarios[MAX_USERS];
    struct Usuario usuarioLogado;
    int numUsuarios = 0;
    int opcao;

    pont_arq = fopen("Users.txt", "r+");
    TratarErrosArquivo(pont_arq);



    if (pont_arq != NULL)
    {
        while (fscanf(pont_arq, "%s %s %d", usuarios[numUsuarios].Login, usuarios[numUsuarios].Senha, &usuarios[numUsuarios].Privilegio) != EOF)
        {
            numUsuarios++;
        }
        fclose(pont_arq);
    }

    pont_arq2 = fopen("Users.txt", "a+");
    TratarErrosArquivo(pont_arq2);

    fseek(pont_arq2, 0, SEEK_END);
    long tamanho = ftell(pont_arq2);

    if (tamanho == 0)
    {
        printf("Nenhum usuário cadastrado\n");
        fprintf(pont_arq2, "admin admin 1 ");
        fclose(pont_arq2);
        pont_arq2 = fopen("Users.txt", "r+");
    }
    else
    {
        fseek(pont_arq2, 0, SEEK_SET);
    }

    do
    {
        printf("Selecione uma opção:\n");
        printf("1. Fazer login\n");
        printf("2. Sair\n");
        printf("Opção: ");
        scanf("%d", &opcao);
        

        switch (opcao)
        {
        case 1:
            usuarioLogado = VerificarLogin(usuarios, numUsuarios);
            menu(usuarios, numUsuarios, usuarioLogado);
            break;
        case 2:
            VerificarLogin(usuarios, numUsuarios);
            printf("Saindo...\n");
            break;
        default:
            printf("Opção inválida. Tente novamente.\n");
        }

    } while (opcao != 2);

    fclose(pont_arq2);
    return 0;
}

void cadastroProd(Produto produtos[], int numProdutos)
{
    // Cadastro de produto

    if (numProdutos < SIZE)
    {
        char nomeNovoProduto[size];

        // Solicitar o nome do produto
        printf(WHT"Digite o nome do produto: "WHT);
        scanf(" %29[^\n]", nomeNovoProduto);

        // Verificar se o produto já existe
        for (int i = 0; i < numProdutos; i++)
        {
            if (strcmp(produtos[i].nome, nomeNovoProduto) == 0)
            {
                printf(RED"Produto já cadastrado!\n"WHT);
                return; // Sair da função se o produto já existe
            }
        }

        // Se o produto não existe, continuar o cadastro
        strcpy(produtos[numProdutos].nome, nomeNovoProduto);

        printf(WHT"Digite a quantidade em estoque: "WHT);
        scanf("%d", &produtos[numProdutos].quantidade);

        produtos[numProdutos].ativo = 1;
        produtos[numProdutos].id = numProdutos + 1;

        printf(GRN"Produto cadastrado com sucesso!\n"WHT);
    }
    else
    {
        printf(RED"Limite de produtos cadastrados atingido!\n"WHT);
    }
}


void checagemEstoque(Produto produtos[], int numProdutos, Compra compras[], int numCompras)
{
    // Carrega os produtos do arquivo antes de exibir o estoque
    int numProdutosCarregados;
    carregarProdutos(produtos, &numProdutosCarregados);

    // Seu codigo de checagem de estoque aqui...

    // Suponhamos que você queira apenas listar os produtos no estoque
    printf("Produtos em estoque:\n");

    for (int i = 0; i < numProdutosCarregados; i++)
    {
        if (produtos[i].ativo)
        {
            printf("%d - %s (ID: %d, Quantidade: %d)\n", i + 1, produtos[i].nome, produtos[i].id, produtos[i].quantidade);
        }
    }
}

int cadastroEstoque(Compra compras[], int numCompras, Produto produtos[], int numProdutos)
{
    int idProduto;
    int dia, mes, ano;

    if (numCompras < SIZE)
    {
        printf("Produtos em estoque:\n");
        for (int i = 0; i < numProdutos; i++)
        {
            if (produtos[i].ativo)
            {
                printf("%d - %s (Quantidade: %d)\n", i + 1, produtos[i].nome, produtos[i].quantidade);
            }
        }

        printf("Escolha o produto (digite o número correspondente): ");
        scanf("%d", &idProduto);

        if (idProduto >= 1 && idProduto <= numProdutos && produtos[idProduto - 1].ativo)
        {
            printf("Digite o custo do produto: ");
            scanf("%f", &compras[numCompras].custo);

            printf("Digite a quantidade comprada: ");
            scanf("%d", &compras[numCompras].quantidade);

            compras[numCompras].id_produto = idProduto;

            produtos[idProduto - 1].quantidade += compras[numCompras].quantidade;

            lerData(&dia, &mes, &ano);

            compras[numCompras].data.dia = dia;
            compras[numCompras].data.mes = mes;
            compras[numCompras].data.ano = ano;

            printf("Data válida: %02d/%02d/%04d\n", dia, mes, ano);

            // Atualizar a quantidade no arquivo "produtos.txt"
            salvarProdutos(produtos, numProdutos);

            numCompras++;
        }
        else
        {
            printf("ID de produto inválido ou produto inativo!\n");
        }
    }
    else
    {
        printf("Limite de compras cadastradas atingido!\n");
    }

    salvarCompras(compras, numCompras, produtos, numProdutos);
    return numCompras;
}


void historicoCusto(Compra compras[], int numCompras)
{
    int mesDesejado, totalComprasNoMes = 0;
    float custoTotalNoMes = 0;

    printf("Digite o mês desejado para o histórico de custo: ");
    scanf("%d", &mesDesejado);

    printf("Histórico de custo para o mês %d:\n", mesDesejado);

    for (int i = 0; i < numCompras; i++)
    {
        if (compras[i].data.mes == mesDesejado)
        {
            printf("ID do Produto: %d\n", compras[i].id_produto);
            printf("Custo: %.2f\n", compras[i].custo);
            printf("Quantidade: %d\n", compras[i].quantidade);
            printf("Data da Compra: %d/%d/%d\n\n", compras[i].data.dia, compras[i].data.mes, compras[i].data.ano);

            custoTotalNoMes += compras[i].custo * compras[i].quantidade;
            totalComprasNoMes += compras[i].quantidade;
        }
    }

    if (totalComprasNoMes > 0)
    {
        float mediaCusto = custoTotalNoMes / totalComprasNoMes;
        printf("Média de custo no mês %d: %.2f\n", mesDesejado, mediaCusto);
    }
    else
    {
        printf(YEL"Nenhuma compra encontrada para o mês %d.\n"WHT, mesDesejado);
    }
}

//-------------------------Funções de arquivo-----------------------------

/// Produtos :: simulação de um BD (salvar)
void salvarProdutos(Produto produtos[], int numProdutos)
{
    FILE *arquivo;
    arquivo = fopen("produtos.txt", "w");  // Modo "w" para escrita

    if (arquivo == NULL)
    {
        fprintf(stderr, RED"Erro ao abrir o arquivo de produtos\n"WHT);
        return;
    }

    //fprintf(arquivo, "%d\n", numProdutos);
    for (int i = 0; i < numProdutos; i++)
    {
        fprintf(arquivo, "%d %d %d %s", produtos[i].id, produtos[i].quantidade, produtos[i].ativo, produtos[i].nome);

        if(i < (numProdutos-1)){
            fprintf(arquivo, "\n");
        }

    }

    fclose(arquivo);
}

/// Produtos :: simulação de um BD (carregar)
void carregarProdutos(Produto produtos[], int *numProdutos)
{
    FILE *arquivo;
    arquivo = fopen("produtos.txt", "r");  // Modo "r" para leitura

    if (arquivo == NULL)
    {
        fprintf(stderr, YEL"Arquivo de produtos não encontrado. Criando novo...\n"WHT);
        *numProdutos = 0;  // Retorna 0 produtos se o arquivo nãoo existir
        return;
    }

    //fscanf(arquivo, "%d", &x);



    /*while (fscanf(arquivo, "%d %s %d %d", &produtos[*numProdutos].id, produtos[*numProdutos].nome, &produtos[*numProdutos].quantidade, &produtos[*numProdutos].ativo) == 4)
    {
        (*numProdutos)++;
        if (*numProdutos >= SIZE)
        {
            fprintf(stderr, RED"Limite de produtos alcançado durante a leitura\n"WHT);
            break;
        }
    }*/

    int i = 0;

    while(!feof(arquivo)){
        fscanf(arquivo, "%d %d %d", &produtos[i].id, &produtos[i].quantidade, &produtos[i].ativo);
        fscanf(arquivo, "%29[^\n]", produtos[i].nome);

        i++;
    }
    *numProdutos = i;



    if (fclose(arquivo) == EOF)
    {
        fprintf(stderr, RED"Erro ao fechar o arquivo de produtos\n"WHT);
    }
}


//---------------------- funções para salvar e carregar estoque ------------------
/// Estoque :: simulando um BD (salvando)
void salvarComprasNaDB(Compra compras[], int numCompras) {
    FILE *arquivo;
    arquivo = fopen("comprasBD.txt", "w");

    if (arquivo == NULL) {
        fprintf(stderr, RED"Erro ao abrir o arquivo de compras\n"WHT);
        return;
    }

    for (int i = 0; i < numCompras; i++) {

        fprintf(arquivo, "%d %f %d %d %d %d",
                compras[i].id_produto,
                compras[i].custo,
                compras[i].quantidade,
                compras[i].data.dia,
                compras[i].data.mes,
                compras[i].data.ano);

        if(i< numCompras-1) fprintf(arquivo, "\n");

    }


    fclose(arquivo);
}

/// Estoque :: simulando um BD (recuperando)
void carregarComprasDaDB(Compra compras[], int *numCompras) {
    FILE *arquivo;
    arquivo = fopen("comprasBD.txt", "r");

    if (arquivo == NULL) {
        fprintf(stderr, RED"Erro ao abrir o arquivo de compras\n"WHT);
        return;
    }

    int i=0;
    while (!feof(arquivo)) {

        fscanf(arquivo, "%d %f %d %d %d %d",
                &compras[i].id_produto,
                &compras[i].custo,
                &compras[i].quantidade,
                &compras[i].data.dia,
                &compras[i].data.mes,
                &compras[i].data.ano);
        i++;
    }

    *numCompras = i;


    fclose(arquivo);
}





//---------------------- salva as compras e faz uma tabulação---------------------

void salvarCompras(Compra compras[], int numCompras, Produto produtos[], int numProdutos) {
    FILE *arquivo;
    arquivo = fopen("compras.txt", "w");

    if (arquivo == NULL) {
        fprintf(stderr, RED"Erro ao abrir o arquivo de compras\n"WHT);
        return;
    }

    fprintf(arquivo, "ID Produto | Nome Produto | Custo Unitário | Quantidade | Data\n");

    // Dicionário para armazenar os totais gastos por mês
    float totalPorMes[12] = {0};

    for (int i = 0; i < numCompras; i++) {
        // Obtemos o índice do produto
        int indiceProduto = compras[i].id_produto - 1;

        fprintf(arquivo, "%-11d | %-13s | %-15.2f | %-10d | %02d/%02d/%04d\n",
                compras[i].id_produto,
                produtos[indiceProduto].nome,
                compras[i].custo,
                compras[i].quantidade,
                compras[i].data.dia,
                compras[i].data.mes,
                compras[i].data.ano);

        // Atualizamos o total gasto por mês
        totalPorMes[compras[i].data.mes - 1] += compras[i].custo * compras[i].quantidade;
    }

    fprintf(arquivo, "\nTotais gastos por mês:\n");
    for (int i = 0; i < 12; i++) {
        fprintf(arquivo, "Mês %02d: %.2f\n", i + 1, totalPorMes[i]);
    }

    fclose(arquivo);
}


void salvarComprasPorAno(Compra compras[], int numCompras, Produto produtos[], int numProdutos, float totalPorMes[])
{
    FILE *arquivo = NULL;
    int anoAtual = 0;

    if (numCompras == 0)
    {
        return; // Não há compras para salvar
    }

    // Fecha o arquivo anterior se estiver aberto
    if (arquivo != NULL)
    {
        fclose(arquivo);
    }

    for (int i = 0; i < numCompras; i++)
    {
        if (compras[i].data.ano != anoAtual)
        {


            // Abre um novo arquivo para o ano atual
            char nomeArquivo[20]; // Tamanho suficiente para armazenar "compras_XXXX.txt"
            sprintf(nomeArquivo, "compras_%d.txt", compras[i].data.ano);
            arquivo = fopen(nomeArquivo, "w"); // Modo "append" (acrescentar)

            if (arquivo == NULL)
            {
                fprintf(stderr, "Erro ao abrir o arquivo de compras para o ano %d\n", compras[i].data.ano);
                return;
            }

            anoAtual = compras[i].data.ano;
        }

        // Escreve a compra no arquivo atual
        fprintf(arquivo, "%-11d | %-13s | %-15.2f | %-10d | %02d/%02d/%04d\n",
                compras[i].id_produto,
                produtos[compras[i].id_produto - 1].nome,
                compras[i].custo,
                compras[i].quantidade,
                compras[i].data.dia,
                compras[i].data.mes,
                compras[i].data.ano);

        // Atualiza o totalPorMes
        totalPorMes[compras[i].data.mes - 1] += compras[i].custo * compras[i].quantidade;
    }

    // Adiciona os totais por mês no arquivo no final
    if (arquivo != NULL)
    {
        fprintf(arquivo, "\nTotais gastos por mês:\n");
        for (int i = 0; i < 12; i++)
        {
            fprintf(arquivo, "Mês %02d: %.2f\n", i + 1, totalPorMes[i]);
        }

        fclose(arquivo);
    }
}



// Função para calcular o total gasto em cada mês
void calcularTotalPorMes(Compra compras[], int numCompras, float totalPorMes[])
{
    for (int i = 0; i < numCompras; i++)
    {
        totalPorMes[compras[i].data.mes - 1] += compras[i].custo * compras[i].quantidade;
    }
}


//-------------------------Funções de ativação de
void desativarProduto(Produto produtos[], int numProdutos)
{
    // Carregar produtos do arquivo antes de exibir a lista
    int numProdutosCarregados;
    carregarProdutos(produtos, &numProdutosCarregados);

    // Mostrar a lista de produtos ativados
    printf("Lista de produtos ativados:\n");
    for (int j = 0; j < numProdutosCarregados; j++)
    {
        if (produtos[j].ativo)
        {
            printf("ID: %d, Nome: %s\n", produtos[j].id, produtos[j].nome);
        }
    }

    int idProduto;
    printf("\nDigite o ID do produto que deseja desativar: ");
    scanf("%d", &idProduto);

    // Verificar se o ID do produto é válido
    int idValido = 0;
    for (int i = 0; i < numProdutosCarregados; i++)
    {
        if (produtos[i].id == idProduto)
        {
            idValido = 1;

            if (produtos[i].ativo == 0) {
                printf(YEL"Produto já está desativado!\n"WHT);
            } else {
                produtos[i].ativo = 0;
                printf(GRN"Produto desativado com sucesso!\n"WHT);

                // Salvar produtos após a alteração
                salvarProdutos(produtos, numProdutosCarregados);
            }

            break;
        }
    }

    if (!idValido)
    {
        printf(RED"ID de produto não encontrado!\n"WHT);
    }
}

void ativarProduto(Produto produtos[], int numProdutos)
{
    // Carregar produtos do arquivo antes de exibir a lista
    int numProdutosCarregados;
    carregarProdutos(produtos, &numProdutosCarregados);

    // Mostrar a lista de produtos desativados
    printf("Lista de produtos desativados:\n");
    for (int j = 0; j < numProdutosCarregados; j++)
    {
        if (!produtos[j].ativo)
        {
            printf("ID: %d, Nome: %s\n", produtos[j].id, produtos[j].nome);
        }
    }

    int idProduto;
    printf("\nDigite o ID do produto que deseja ativar: ");
    scanf("%d", &idProduto);

    // Verificar se o ID do produto é válido
    int idValido = 0;
    for (int i = 0; i < numProdutosCarregados; i++)
    {
        if (produtos[i].id == idProduto)
        {
            idValido = 1;

            if (produtos[i].ativo == 1) {
                printf(YEL"Produto já está ativado!\n"WHT);
            } else {
                produtos[i].ativo = 1; // Ativa o produto
                printf(GRN"Produto ativado com sucesso!\n"WHT);

                // Salvar produtos após a alteração
                salvarProdutos(produtos, numProdutosCarregados);
            }

            break;
        }
    }

    if (!idValido)
    {
        printf(RED"ID de produto não encontrado!\n"WHT);
    }
}


int validarData(int dia, int mes, int ano)
{
    if (mes < 1 || mes > 12 || dia < 1 || dia > 31)
    {
        return 0; // Data inválida
    }

    // Verifique os meses com 30 dias
    if ((mes == 4 || mes == 6 || mes == 9 || mes == 11) && dia > 30)
    {
        return 0; // Data inválida
    }

    // Verifique o mês de fevereiro e anos bissextos
    if (mes == 2)
    {
        if ((ano % 4 == 0 && ano % 100 != 0) || (ano % 400 == 0))
        {
            if (dia > 29)
            {
                return 0; // Data inválida
            }
        }
        else
        {
            if (dia > 28)
            {
                return 0; // Data inválida
            }
        }
    }

    return 1; // Data válida
}



int lerData(int *dia, int *mes, int *ano) {
    int diaDigitado, mesDigitado, anoDigitado;

    do {
        printf("Digite o dia: ");
        if (scanf("%d", &diaDigitado) != 1) {
            fprintf(stderr, "Erro ao ler o dia. Certifique-se de inserir um número.\n");
            return 0;
        }

        printf("Digite o mês: ");
        if (scanf("%d", &mesDigitado) != 1) {
            fprintf(stderr, "Erro ao ler o mês. Certifique-se de inserir um número.\n");
            return 0;
        }

        printf("Digite o ano: ");
        if (scanf("%d", &anoDigitado) != 1) {
            fprintf(stderr, "Erro ao ler o ano. Certifique-se de inserir um número.\n");
            return 0;
        }

        if (!validarData(diaDigitado, mesDigitado, anoDigitado)) {
            printf("Data inválida. Por favor, tente novamente.\n");
        } else {
            // Se todas as partes da data são válidas, atribua aos ponteiros
            *dia = diaDigitado;
            *mes = mesDigitado;
            *ano = anoDigitado;
        }

        // Limpa o buffer de entrada
        int c;
        while ((c = getchar()) != '\n' && c != EOF);

    } while (!validarData(*dia, *mes, *ano));

    return 1; // Data válida
}



void saidaEstoque(Produto produtos[], int numProdutos)
{
    int idProduto, quantidadeSaida;

    printf("Produtos em estoque:\n");
    for (int i = 0; i < numProdutos; i++)
    {
        printf("%d - %s (Quantidade: %d)\n", produtos[i].id, produtos[i].nome, produtos[i].quantidade);
    }

    printf("Escolha o produto (digite o número correspondente): ");
    scanf("%d", &idProduto);

    if (idProduto >= 1 && idProduto <= numProdutos && produtos[idProduto - 1].ativo)
    {
        printf("Digite a quantidade de saída: ");
        scanf("%d", &quantidadeSaida);

        if (quantidadeSaida <= produtos[idProduto - 1].quantidade)
        {
            produtos[idProduto - 1].quantidade -= quantidadeSaida;

            registrarSaida(produtos[idProduto - 1].nome, quantidadeSaida);

            printf("Saída de estoque registrada com sucesso!\n");
        }
        else
        {
            printf("Quantidade insuficiente em estoque!\n");
        }
    }
    else
    {
        printf("ID de produto inválido ou produto inativo!\n");
    }
}

void registrarSaida(const char *nomeProduto, int quantidadeSaida)
{
    FILE *arquivo;
    arquivo = fopen("saida.txt", "a");  // Abre o arquivo no modo "append" (acrescentar)

    if (arquivo == NULL)
    {
        fprintf(stderr, "Erro ao abrir o arquivo de compras\n");
        return;
    }

    // Obtém a data atual
    time_t t;
    struct tm *dataAtual;

    t = time(NULL);
    dataAtual = localtime(&t);

    // Escreve a saída no arquivo
    fprintf(arquivo, "- | %-13s | - | %-10d | %02d/%02d/%04d\n",
            nomeProduto,
            quantidadeSaida,
            dataAtual->tm_mday,
            dataAtual->tm_mon + 1,
            dataAtual->tm_year + 1900);

    fclose(arquivo);
}

//-------------------------------Funções de Login -------------------------------------

void cadastro(struct Usuario novoUsuario, struct Usuario *usuarios, int *numUsuarios)
{
    clearScrean();

    for (int i = 0; i < *numUsuarios; i++)
    {
        if (strcmp(novoUsuario.Login, usuarios[i].Login) == 0)
        {
            printf("Erro: Nome de usu�rio j� em uso. Escolha outro.\n");
         pause();
            clearScrean();
            return;
        }
    }

    // Adiciona o novo usu�rio ao arquivo
    fprintf(pont_arq2, "%s %s %d\n", novoUsuario.Login, novoUsuario.Senha, novoUsuario.Privilegio);
    printf("Usu�rio cadastrado com sucesso!\n");

    fseek(pont_arq2, 0, SEEK_SET);

    int i = 0;
    while (fscanf(pont_arq2, "%s %s", usuarios[i].Login, usuarios[i].Senha) != EOF)
    {
        i++;
    }
    (*numUsuarios)++;

 pause();
    clearScrean();
}

struct Usuario VerificarLogin(struct Usuario usuarios[], int numUsuarios)
{
    char Login[MAX_USERNAME];
    char Senha[MAX_PASSWORD];
    int i;
    Confirma = 0;

    clearScrean();

    fseek(pont_arq2, 0, SEEK_SET);

    while (!Confirma)
    {
        printf("Digite o nome de usu�rio: ");
        scanf("%s", Login);

        // Adiciona verifica��o de limite para o nome de usu�rio
        if (strlen(Login) >= MAX_USERNAME)
        {
            printf("Nome de usu�rio muito longo. Tente novamente.\n");
            clearScrean();
            continue;
        }

        printf("Digite a senha: ");
        scanf("%s", Senha);

        // Adiciona verifica��o de limite para a senha
        if (strlen(Senha) >= MAX_PASSWORD)
        {
            printf("Senha muito longa. Tente novamente.\n");
            clearScrean();
            continue;
        }

        for (i = 0; i < numUsuarios; i++)
        {
            if (strcmp(Login, usuarios[i].Login) == 0)
            {
                // Se a senha for "admin", n�o criptografa
                if (strcmp(Senha, "admin") == 0)
                {
                    printf("Login bem-sucedido! N�vel de privil�gio: %d\n", usuarios[i].Privilegio);
                    printf("Login bem-sucedido!\n");
                    Confirma = 1;
                }
                else
                {
                    // Caso contr�rio, criptografa a senha e verifica
                    CriptografarSenha(Senha);
                    if (strcmp(Senha, usuarios[i].Senha) == 0)
                    {
                        printf("Login bem-sucedido! N�vel de privil�gio: %d\n", usuarios[i].Privilegio);
                        Confirma = 1;
                    }
                }

                if (Confirma)
                {

                    pause();
                    clearScrean();
                    return usuarios[i];
                }
                else
                {
                    printf("Usu�rio ou senha incorretos. Tente novamente.\n");
                }
                clearScrean();
            }
        }

        if (!Confirma)
        {
            clearScrean();
            printf("Usu�rio ou senha incorretos. Tente novamente.\n");
         pause();
        }

        clearScrean();
    }
    return usuarios[i];
}

void menu(struct Usuario usuarios[], int numUsuarios, struct Usuario usuarioLogado)
{
    limparBuffer();
    char sair;
    int escolha, continuar = 1, numProdutos = 0, numCompras = 0;
    float totalPorMes[12] = {0};
    Produto produtos[SIZE];
    Compra compras[SIZE];

    // Adicione esta chamada para carregar os produtos
    carregarProdutos(produtos, &numProdutos);

    carregarComprasDaDB(compras, &numCompras);

  //  printf("LISTAGEM DAS COMPRAS\n");

   // for(int i =0; i<numCompras; i++){
   //     printf("%d %d %.2f %d/%d/%d\n", compras[i].id_produto, compras[i].quantidade, compras[i].custo, compras[i].data.dia, compras[i].data.mes, compras[i].data.ano);
    //}

    while (continuar == 1)
    {

        printf(RED"\n ========== ");
        printf(BLU BLKB"Menu de Opções"WHT);
        printf(RED" =============\n");
        printf(RED" |");
        printf(textoMenu"1 - Cadastro de Produto              ");
        printf(RED"|\n ");
        printf(RED"|");
        printf(textoMenu"2 - Checagem de Estoque              ");
        printf(RED"|\n ");
        printf(RED"|");
        printf(textoMenu"3 - Cadastro de Estoque              ");
        printf(RED"|\n ");
        printf(RED"|");
        printf(textoMenu"4 - Desativar Produto                ");
        printf(RED"|\n ");
        printf(RED"|");
        printf(textoMenu"5 - Ativar Produto                   ");
        printf(RED"|\n ");
        printf(RED"|");
        printf(textoMenu"6 - Opções de Usuario           ");
        printf(RED"|\n ");
        printf(RED"|");
        printf(textoMenu"7 - Saida de estoque                 ");
        printf(RED"|\n ");
        printf(RED"|");
        printf(textoMenu"8 - Sair                             ");
        printf(RED"|\n ");
        printf("=======================================\n");

        printf(WHT"Digite a opção desejada: "WHT);
        scanf("%d", &escolha);

        // Limpar o buffer de entrada
        int c;
        while ((c = getchar()) != '\n' && c != EOF);

        system("cls");

        switch (escolha)
        {
        case 1:
            printf("1 - Cadastro de produto\n");
            cadastroProd(produtos, numProdutos);
            numProdutos++;
            // Salvar produtos após cada cadastro
            salvarProdutos(produtos, numProdutos);
            break;

        case 2:
            printf("2 - Checagem de estoque\n");
            checagemEstoque(produtos, numProdutos, compras, numCompras);
            break;

        case 3:
            printf("3 - Cadastro de estoque\n");
            numCompras = cadastroEstoque(compras, numCompras, produtos, numProdutos);
            // Altere a chamada para incluir totalPorMes como argumento
            salvarComprasPorAno(compras, numCompras, produtos, numProdutos, totalPorMes);
            break;

        case 4:
            printf("4 - Desativar produto\n");
            desativarProduto(produtos, numProdutos);
            break;

        case 5:
            printf("5 - Ativar produto\n");
            ativarProduto(produtos, numProdutos);
            break;

        case 6:
            printf("6 - Opções de Usuario");
            OpcoesDeUsuario(usuarios, numUsuarios);
            break;

        case 7:
            printf("7 - Saida de produto\n");
            saidaEstoque(produtos,numProdutos);

            // Salvar produto pós salvação

            salvarProdutos(produtos, numProdutos);
            break;

        case 8:
            printf("Deseja realmente sair? (S/N)\n");
            scanf(" %c", &sair);
            sair = toupper(sair);
            if (sair == 'S')
            {
                continuar = 0;
            }
            salvarComprasNaDB(compras, numCompras);
            break;

        default:
            printf("Opção Inválida!!!\n");
            break;
        }
    }
}

void EditarUsuario(struct Usuario usuarios[], int numUsuarios)
{
    char Login[MAX_USERNAME];
    char NovoLogin[MAX_USERNAME];
    char NovaSenha[MAX_PASSWORD];
    int novoPrivilegio;
    int i;

    clearScrean();

    printf("Digite o nome de usu�rio que deseja editar: ");
    scanf("%s", Login);

    // Adiciona verifica��o de limite para o nome de usu�rio
    if (strlen(Login) >= MAX_USERNAME)
    {
        printf("Nome de usu�rio muito longo. Tente novamente.\n");
     pause();
        clearScrean();
        return;
    }

    for (i = 0; i < numUsuarios; i++)
    {
        if (strcmp(Login, usuarios[i].Login) == 0)
        {
            printf("Novo nome de usu�rio: ");
            scanf("%s", NovoLogin);

            // Adiciona verifica��o de limite para o novo nome de usu�rio
            if (strlen(NovoLogin) >= MAX_USERNAME)
            {
                printf("Novo nome de usu�rio muito longo. Tente novamente.\n");
             pause();
                clearScrean();
                return;
            }

            printf("Nova senha: ");
            scanf("%s", NovaSenha);

            // Adiciona verifica��o de limite para a nova senha
            if (strlen(NovaSenha) >= MAX_PASSWORD)
            {
                printf("Nova senha muito longa. Tente novamente.\n");
             pause();
                clearScrean();
                return;
            }

            printf("Novo Privilegio: ");
            scanf("%d", &novoPrivilegio);

            // Atualiza as informa��es do usu�rio
            strcpy(usuarios[i].Login, NovoLogin);
            CriptografarSenha(NovaSenha);
            strcpy(usuarios[i].Senha, NovaSenha);
            usuarios[i].Privilegio = novoPrivilegio;

            printf("Usu�rio editado com sucesso!\n");

            // Abre o arquivo para escrita e salva as altera��es
            FILE *temp_arq = fopen("temp.txt", "w");
            TratarErrosArquivo(temp_arq);

            for (int k = 0; k < numUsuarios; k++)
            {
                fprintf(temp_arq, "%s %s %d\n", usuarios[k].Login, usuarios[k].Senha, usuarios[k].Privilegio);
            }

            fclose(temp_arq);
            fclose(pont_arq2);

            // Remove o arquivo original
            remove("Users.txt");

            // Renomeia o arquivo tempor�rio para o original
            rename("temp.txt", "Users.txt");

         pause();
            clearScrean();
            break; // Sai do loop ap�s editar o usu�rio
        }
    }

    if (i == numUsuarios)
    {
        printf("Usu�rio n�o encontrado. Tente novamente.\n");
    }

}

int UsuarioExiste(struct Usuario usuarios[], int numUsuarios, const char *login)
{
    for (int i = 0; i < numUsuarios; i++)
    {
        if (strcmp(login, usuarios[i].Login) == 0)
        {
            return 1; // Retorna 1 se o usu�rio existe
        }
    }
    return 0; // Retorna 0 se o usu�rio n�o existe
}

void DeletarUsuario(struct Usuario usuarios[], int *numUsuarios)
{
    char Login[MAX_USERNAME];
    int i, j;

    clearScrean();

    printf("Digite o nome de usu�rio que deseja deletar: ");
    scanf("%s", Login);

    // Verifica se o usu�rio existe antes de tentar excluí-lo
    if (!UsuarioExiste(usuarios, *numUsuarios, Login))
    {
        printf("Usu�rio n�o encontrado. Tente novamente.\n");
     pause();
        clearScrean();

    }

    for (i = 0; i < *numUsuarios; i++)
    {
        if (strcmp(Login, usuarios[i].Login) == 0)
        {
            // Move os usu�rios seguintes para preencher o espa�o do usu�rio deletado
            for (j = i; j < *numUsuarios - 1; j++)
            {
                strcpy(usuarios[j].Login, usuarios[j + 1].Login);
                strcpy(usuarios[j].Senha, usuarios[j + 1].Senha);
            }

            (*numUsuarios)--; // Decrementa o n�mero total de usu�rios
            fclose(pont_arq2);

            // Remove o arquivo original
            remove("Users.txt");

            // Cria um novo arquivo
            pont_arq2 = fopen("Users.txt", "w");

            // Escreve os usu�rios restantes no novo arquivo
            for (int k = 0; k < *numUsuarios; k++)
            {
                fprintf(pont_arq2, "%s %s\n", usuarios[k].Login, usuarios[k].Senha);
            }

            fclose(pont_arq2);

            printf("Usu�rio deletado com sucesso!\n");
         pause();
            clearScrean();
            break; // Sai do loop ap�s deletar o usu�rio
        }
    }

    return;
}

void OpcoesDeUsuario(struct Usuario usuarios[], int *numUsuarios)
{
    int opcoes1 = 0;

    setbuf(stdin, NULL);
    clearScrean();

    printf("=================================================\n");
    printf("|             Op��es de Usu�rio                 |\n");
    printf("=================================================\n");
    printf("| 1-Cadastrar novo usu�rio                      |\n");
    printf("-------------------------------------------------\n");
    printf("| 2-Editar Usu�rios                             |\n");
    printf("-------------------------------------------------\n");
    printf("| 3-Deletar Usu�rio                             |\n");
    printf("=================================================\n");
    printf("Selecione uma opção: ");
    scanf("%d", &opcoes1);



    switch (opcoes1)
    {
    case 1:
        clearScrean();
        printf("1-Cadastrar novo usuario\n");
        // Verifica se o limite m�ximo de usu�rios foi atingido
        if (*numUsuarios >= MAX_USERS)
        {
            printf("Erro: Limite m�ximo de usu�rios atingido.\n");
         pause();
            clearScrean();
            break;
        }

        printf("Digite o nome de usu�rio: ");
        char novoLogin[MAX_USERNAME];
        scanf("%s", novoLogin);

        // Verifica se o usu�rio existe antes de tentar excluí-lo
        if (UsuarioExiste(usuarios, *numUsuarios, novoLogin))
        {
            printf("Erro: Nome de usu�rio j� em uso. Escolha outro.\n");
         pause();
            clearScrean();
            return;
        }

        if (strlen(novoLogin) >= MAX_USERNAME)
        {
            printf("Erro: Nome de usu�rio muito longo.\n");
         pause();
            clearScrean();
            break;
        }

        printf("Digite a senha: ");
        char novaSenha[MAX_PASSWORD];
        scanf("%s", novaSenha);

        // Valida��o de entrada
        if (strlen(novaSenha) >= MAX_PASSWORD)
        {
            printf("Erro: Senha muito longa.\n");
         pause();
            clearScrean();
            break;
        }

        printf("Digite o level de privilegio: ");
        int novoPrivilegio;
        scanf("%d", &novoPrivilegio);

        CriptografarSenha(novaSenha);
        strcpy(usuarios[*numUsuarios].Login, novoLogin);
        strcpy(usuarios[*numUsuarios].Senha, novaSenha);
        usuarios[*numUsuarios].Privilegio = novoPrivilegio;
        cadastro(usuarios[*numUsuarios], usuarios, numUsuarios);

        break;

    case 2:
        clearScrean();
        printf("2 - Editar usu�rio\n");
        EditarUsuario(usuarios, *numUsuarios);
        break;

    case 3:
        DeletarUsuario(usuarios, numUsuarios);
        break;

    default:
        clearScrean();
        printf("Op��o Inv�lida!!!\n");
     pause();

        break;
    }
    limparBuffer();
}

void TratarErrosArquivo(FILE *pont_arq)
{
    if (pont_arq == NULL)
    {
        printf("Erro ao abrir o arquivo. O programa ser� encerrado.\n");
        exit(EXIT_FAILURE);
    }
}

void CriptografarSenha(char *senha)
{
    // Soma os valores ASCII dos caracteres da senha
    int i = 0;
    while (senha[i] != '\0')
    {
        senha[i] = senha[i] + 5; // Adiciona um valor arbitr�rio (5) como exemplo
        i++;
    }
}


void clearScrean() //adiciona a fun��o de limpar a tela
{
    int n;
    for (n = 0; n < 10; n++)
        printf( "\n\n\n\n\n\n\n\n\n\n" );
}

void pause() //adiciona a fun��o de pausa ao codigo
{
    setbuf(stdin,NULL);
    printf("Pressione qualquer tecla para continuar...");
    getchar();
}

void limparBuffer()
{

    setbuf(stdin, NULL);

}

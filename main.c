#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

char *get_path(char *);

int main(void) {
  char *comando = NULL, *comandocp = NULL;
  char *token;
  char dir[1024] = "";
  char **argumentos;
  size_t tamanho;
  const char *separador = " \n";
  int exit_flag = 0;
  int cd_flag = 0;
  const char *saida = "exit\n";
  const char *cd = "cd";
  pid_t cpid;
  getcwd(dir, sizeof(dir));

  printf("##    Starting Shell    ##\n\n");
  while (exit_flag == 0) {

    // executa o laço apenas quando o filho retornar
    cpid = wait(NULL);

    // le linha e cria string, aloca copia de mesmo tamanho
    printf("%s/ (Shell)$ ", dir);
    cd_flag = 0;
    exit_flag = getline(&comando, &tamanho, stdin);

    // testa condição de saida
    if (!strcmp(comando, saida) || exit_flag == -1) {
      exit_flag = 1;
    } else {
      exit_flag = 0;
    }

    // strtok usando a copia
    comandocp = strdup(comando);
    token = strtok(comandocp, separador);

    // laço que cria o tamanho de tokens
    int num_tokens = 0;
    while (token != NULL) {
      num_tokens++;
      token = strtok(NULL, separador);
    }
    // printf(" ~ Numero de tokens: %d\n", num_tokens);

    // aloca o vetor para receber argumentos
    argumentos = malloc(sizeof(char *) * num_tokens);

    // usa copia para strtok
    comandocp = strdup(comando);
    token = strtok(comandocp, separador);

    // laço de crição de argumentos com base no token
    for (int i = 0; token != NULL; i++) {
      argumentos[i] = malloc(sizeof(char) * strlen(token));
      strcpy(argumentos[i], token);
      // printf(" ~ Argumento[%d] = %s \n", i, argumentos[i]);
      token = strtok(NULL, separador);
    }
    argumentos[num_tokens] = NULL; // define o ultimo argumento

    if (!strcmp(argumentos[0], cd)) {
      cd_flag = 1;
      int chidir = chdir(argumentos[1]);
      if (!chidir) {
        getcwd(dir, sizeof(dir));
        // printf("diretorio: %s",dir);
      } else {
        perror("ERROR");
      }
    }
    // checa a condição de saida e cria uma thread
    if (exit_flag == 0 && fork() == 0) {

      // se argumentos existe, acha o path do comando e executa
      if (argumentos && cd_flag == 0) {
        char *file_path = NULL;
        file_path = get_path(argumentos[0]);

        // printf(" ~ Path: %s\n\n", file_path);
        printf("\n");

        // execução
        if (execvp(file_path, argumentos) == -1) {
          perror("Error");
        };
      }
      return (0);
    }
  }

  free(comando);
  free(comandocp);
  free(argumentos);
  printf("\n##    Exiting Shell    ##\n\n");
  return (0);
}

// Função de encontrar o path do comando
char *get_path(char *comando) {
  char *path, *path_token, *file_path;
  int tam_comando, tam_path;
  tam_comando = sizeof(comando);

  // recebe os caminhos do sistema e cria tokens
  path = getenv("PATH");
  path_token = strtok(path, ":");

  // laço para acessar os tokens
  while (path_token != NULL) {
    tam_path = sizeof(path_token);
    file_path = malloc(sizeof(char *) * (tam_comando + tam_path + 1));

    // concatena o caminho com o comando
    strcpy(file_path, path_token);
    strcat(file_path, "/");
    strcat(file_path, comando);

    // checa e retorna se o comando existir
    struct stat stat_path_token;
    if (stat(file_path, &stat_path_token) == 0) {
      return file_path;
    }
    path_token = strtok(NULL, ":");
  }

  // retorna o proprio comando se não houver caminho
  return comando;
}
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define TAILLE_BUFF 1024
#define MAX_ARGS 100

// Fonction pour organiser la commande dans le tableau cmd
int organiser_cmd(char *buff, char cmd[MAX_ARGS][MAX_ARGS]) {
    int i = 0;
    //recuperer ma chaine de caractere receuilli en entrer standar et la decoupe en mots
    char *token = strtok(buff, " \n");

    while (token != NULL && i < MAX_ARGS) {
        strncpy(cmd[i], token, MAX_ARGS - 1); // Copie les max_args-1 ieme caractere qui se trouve a l'addresse renvoye par token , lettre par lettre

        cmd[i][MAX_ARGS - 1] = '\0'; // S'assurer de la terminaison de chaque chaîne si le nombre de caractere du mot est plus grand que max_args-1
        i++;
        printf(" je suis le cmd %s\n", cmd[i-1]);//j'affiche les mots que j'ai eu apres chaqu'appel de strtok
        //printf("je suis le tken %c\n", *token);
        token = strtok(NULL, " \n");//pour continuer ou on s'est arrete dans le decoupage
    }
    return i; // Retourne le n ombre d'arguments trouvés
}

// Fonction pour exécuter une commande interne
void executer_commande_interne(char cmd[MAX_ARGS][MAX_ARGS], int nb_args) {
    //on verifie si le premier caractere correspond a un soit sortir , soit changer de repetoir
    if (strcmp(cmd[0], "exit") == 0) {
        printf("Fermeture du shell.....\n");
        exit(0);
    } else if (strcmp(cmd[0], "cd") == 0) {
        if (nb_args < 2) {
            fprintf(stderr, "Erreur : chemin requis pour 'cd'.\n");
        } else if (chdir(cmd[1]) != 0) {
            perror("Erreur de changement de répertoire");
        }
    } else {
        fprintf(stderr, "Commande interne non reconnue : %s\n", cmd[0]);
    }
}

// Fonction pour interpréter et exécuter la commande
void interpreter_executer_cmd(char cmd[MAX_ARGS][MAX_ARGS], int nb_args) {
    // Si aucune commande n'a été saisie, on sort
    if (nb_args == 0) return;

    // Convertir cmd en un tableau de pointeurs pour execvp et pour gagner en espace
    char *args[MAX_ARGS];
    for (int i = 0; i < nb_args; i++) {
        args[i] = cmd[i];//cmd c'est
    }
    args[nb_args] = NULL; // Fin du tableau de pointeurs

    // Vérifier si la commande est interne
    if (strcmp(cmd[0], "exit") == 0 || strcmp(cmd[0], "cd") == 0) {
        executer_commande_interne(cmd, nb_args);
    } else {
        // Commande externe, on utilise fork pour exécuter dans le processus fils
        pid_t pid = fork();
        if (pid < 0) {
            perror("Erreur de fork");
            exit(1);
        } else if (pid == 0) {
            // Processus fils : exécuter la commande externe
            if (execvp(args[0], args) == -1) {
                perror("Commande non trouvée ou échec d'exécution");
            }
            exit(1); // Sortie en cas d'échec de execvp
        } else {
            // Processus parent : attendre la fin de l'exécution du fils
            int status;
            waitpid(pid, &status,0);
        }
    }
}

int main() {
    char *buff = (char *)malloc(TAILLE_BUFF); // Allocation dynamique pour stocker la commande
    //printf("buffer %s", *buff);
    if (buff == NULL) {
        perror("Erreur d'allocation mémoire");
        return 1;
    }

    char cmd[MAX_ARGS][MAX_ARGS]; // Tableau pour stocker les arguments séparés

    while (1) {
        printf("\ntest_mini-shell@bala>> ");
        if (fgets(buff, TAILLE_BUFF, stdin) == NULL) {


            break; // Sortie du shell si Ctrl+D est pressé

        }
        /*char chaine [2][6]={"hskjd\0jsjs\0","jkssjhshss\0","kkkkkkkkk","hhhhhhh"};
        printf("\nhello %s",chaine[0]);
        printf("\nje suis  le buffer : %s",buff);*/
        // Organiser la commande dans cmd
        int nb_args = organiser_cmd(buff, cmd);

        // Interpréter et exécuter la commande
        interpreter_executer_cmd(cmd, nb_args);
    }

    free(buff); // Libération de la mémoire
    return 0;
}

#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
 
int main() {
   int head_status;
   pid_t head = fork();
   if (head < 0) {
      fprintf(stderr, "Can't fork, exiting...\n");
      exit(EXIT_FAILURE);
   } else if (head == 0) { // head
      int sort_status;
      int fd1[2]; // pipe: sort --> head
      pipe(fd1);
      pid_t sort = fork();
      if (sort < 0) {
         fprintf(stderr, "Can't fork, exiting...\n");
         exit(EXIT_FAILURE);
      } else if (sort == 0) { // sort
         dup2(fd1[1], STDOUT_FILENO);
         close(fd1[0]); close(fd1[1]);
 
         int awk_status;
         int fd2[2]; // pipe: awk --> sort
         pipe(fd2);
         pid_t awk = fork();
         if (awk < 0) {
            fprintf(stderr, "Can't fork, exiting...\n");
            exit(EXIT_FAILURE);
         } else if (awk == 0) { // awk
            dup2(fd2[1], STDOUT_FILENO);
            close(fd2[0]); close(fd2[1]);
 
        char * argv[] = {"awk", "-F\\\"", "\
$1 ~ /18\\/Oct\\/2006/ && $4 != \"-\" { sum[$4] += 1; s += 1; }\n\
END {\n\
  for(i in sum) {\n\
     print i, \"-\", sum[i], \"-\", sum[i]*100/s, \"%\"\n\
  }\n\
}", "log.txt", 0};
        execvp(argv[0], argv);
        exit(EXIT_FAILURE);
         } else { // sort
            dup2(fd2[0], STDIN_FILENO);
            close(fd2[0]); close(fd2[1]);
 
            waitpid(awk, &awk_status, 0);
 
        char * argv[] = {"sort", "-nrk3", 0};
        execvp(argv[0], argv);
        exit(EXIT_FAILURE);
         }
      } else { // head
         dup2(fd1[0], STDIN_FILENO);
         close(fd1[0]); close(fd1[1]);
 
         waitpid(sort, &sort_status, 0);
 
     char * argv[] = {"head", 0};
     execvp(argv[0], argv);
     exit(EXIT_FAILURE);
      }
   }
   // parent
   waitpid(head, &head_status, 0);
   exit(head_status);
}
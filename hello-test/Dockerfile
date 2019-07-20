FROM gcc:8.3
COPY . /usr/src/myapp
WORKDIR /usr/src/myapp
# gcc -o = compila in un file oggetto, myapp è il nome
# dell'eseguibile che viene creato e main.c è il file
# dal quale viene creato l'eseguibile
RUN gcc -o myapp main.c
CMD ["./myapp"]
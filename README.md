
# Sistema Solar com OpenGL

## 💻 Sobre o projeto
 O projeto consiste em desenhover/desenhar uma cena em 3D com a formação e movimentação do sistema solar interativo utilizando C++ e OpengGL.

## ⚙️ Funcionamento: 

#### Para Compilar:
-  Abrir um terminal na pasta onde os arquivos estão presentes e executar:

```
gcc -c Camera.cpp -o camera.o
```

```
g++ camera.o main.cpp -o teste -lGL -lGLU -lglfw -lrt -lm -ldl -lXrandr -lXext -lXrender -lX11 -lpthread -lXau -lXdmcp -lGLEW -lGLU -lGL -lm -ldl -ldrm  -lXext -lX11 -lpthread -lxcb -lXau -lXdmcp
```
## 🎥 Vídeo demonstrando funcionamento: 

https://www.youtube.com/watch?v=BtSEcBt59vg

## 🛠 Tecnologias Utilizadas

As ferramentas usadas para a construção do projeto foram as seguintes:

- [Visual Studio Code](https://visualstudio.microsoft.com/pt-br/downloads/)
- [MingGW](https://sourceforge.net/projects/mingw/)
- [OPENGL](https://www.opengl.org/)
- [GLFW](https://www.glfw.org/)
- [GLM](https://glm.g-truc.net/0.9.9/)

## 🚧 Dificuldades encontradas 

De inicio a equipe teve dificuldades para configurar o ambiente com as dependências e conseguir dar os primeiros passos no trabalho. Somente três integrantes conseguiram configurar o ambiente, dois utilizando o Visual Studio e um pelo VS Code, e ao avançar no trabalho apresentou-se um erro no Visual Studio referente a inicialização dos projetos e como não foi encontrado solução para não perder foi decidido escolher o VS Code para construir o trabalho. Pensando nos integrantes que não conseguiram confirgurar o ambiente decidimos marcar sempre uma reunião a cada avanço que era obtido no trabalho para explicar como estava sendo feito o projeto e também para possíveis ajustes ou aplicação de novas funções.
Na primeira parte do projeto a maior dificuldade foi importar corretamente as bibliotecas e chamar corretamente elas ao decorrer do programa, outra dificuldade foi a compilação do código com erros na crição da cena, resultando em tela escura, e na rotação dos planetas.
Na segunda parte do trabalho mais dois membros conseguiram configurar corretamente o ambiente e executar o projeto, sendo assim tivemos mais tentativas na aplicação da iluminação, as dificuldades aqui encontradas foi com relação a aplicação da luz posicional, especular e no ambiente que não eram visto diferenças. A equipe tentou predefinir valores e chamar as funções iluminando cada planeta mas ainda assim não foi possivel ver modificação. Por fim, foi encontrado a solução predefinindo valores para luz direcional e intensidade e passando como parâmetro na função de localização de cada planeta. 

## 💪 Equipe

- André Luis Marques Rodrigues - 374866
- André Veras - 385099
- Alisson Vitor Gomes Ferreira - 378979
- Bryan Santos Sousa - 338888
- Clara Beatriz de Sousa Gomez - 430382
- Carlos Eduardo Tabosa Silva - 385101
- Antonio Ray Martins Vieira - 404583

## 📝Referencias

[Mostrar objetos em uma janela](https://cs.lmu.edu/~ray/notes/openglexamples/)
[Rotação dos planetas](https://www.inf.ufrgs.br/~amaciel/teaching/SIS0381-10-1/exercise8.html)
[Aplicação de texturas](https://www.codeincodeblock.com/2012/05/simple-method-for-texture-mapping-on.html)


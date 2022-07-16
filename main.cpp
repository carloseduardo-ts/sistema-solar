
#include <array>
#include <iostream>
#include <fstream>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "deps/stb/stb_image.h"
#include "Camera.h"

int Width = 800;
int Height = 600;

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec3 Color;
	glm::vec2 UV;
};

struct Triangle
{
	GLuint V0;
	GLuint V1;
	GLuint V2;
};

struct DirectionalLight
{
	glm::vec3 Direction;
	GLfloat Intensity;
};

SimpleCamera Camera;

void GenerateSphere(GLuint Resolution, std::vector<Vertex>& Vertices, std::vector<Triangle>& Indices)
{
	Vertices.clear();
	Indices.clear();

	constexpr float Pi = glm::pi<float>();
	constexpr float TwoPi = glm::two_pi<float>();
	float InvResolution = 1.0f / static_cast<float>(Resolution - 1);

	for (GLuint UIndex = 0; UIndex < Resolution; ++UIndex)
	{
		const float U = UIndex * InvResolution;
		const float Theta = glm::mix(0.0f, TwoPi, static_cast<float>(U));

		for (GLuint VIndex = 0; VIndex < Resolution; ++VIndex)
		{
			const float V = VIndex * InvResolution;
			const float Phi = glm::mix(0.0f, Pi, static_cast<float>(V));

			glm::vec3 VertexPosition =
			{
				glm::cos(Theta) * glm::sin(Phi),
				glm::sin(Theta) * glm::sin(Phi),
				glm::cos(Phi)
			};

			glm::vec3 VertexNormal = glm::normalize(VertexPosition);

			Vertices.push_back(Vertex{
				VertexPosition,
				VertexNormal,
				glm::vec3{ 1.0f, 1.0f, 1.0f },
				glm::vec2{ 1.0f - U, 1.0f - V }
			});
		}
	}

	for (GLuint U = 0; U < Resolution - 1; ++U)
	{
		for (GLuint V = 0; V < Resolution - 1; ++V)
		{
			GLuint P0 = U + V * Resolution;
			GLuint P1 = U + 1 + V * Resolution;
			GLuint P2 = U + (V + 1) * Resolution;
			GLuint P3 = U + 1 + (V + 1) * Resolution;

			Indices.push_back(Triangle{ P3, P2, P0 });
			Indices.push_back(Triangle{ P1, P3, P0 });
		}
	}
}

std::string ReadFile(const char* FilePath)
{
	std::string FileContents;
	if (std::ifstream FileStream{ FilePath, std::ios::in })
	{
		FileContents.assign((std::istreambuf_iterator<char>(FileStream)), std::istreambuf_iterator<char>());
	}
	return FileContents;
}

void CheckShader(GLuint ShaderId)
{
	// Verificar se o shader foi compilado
	GLint Result = GL_TRUE;
	glGetShaderiv(ShaderId, GL_COMPILE_STATUS, &Result);

	if (Result == GL_FALSE)
	{
		// Erro ao compilar o shader, imprimir o log para saber o que est� errado
		GLint InfoLogLength = 0;
		glGetShaderiv(ShaderId, GL_INFO_LOG_LENGTH, &InfoLogLength);

		std::string ShaderInfoLog(InfoLogLength, '\0');
		glGetShaderInfoLog(ShaderId, InfoLogLength, nullptr, &ShaderInfoLog[0]);

		if (InfoLogLength > 0)
		{
			std::cout << "Erro no Vertex Shader: " << std::endl;
			std::cout << ShaderInfoLog << std::endl;

			assert(false);
		}
	}
}

GLuint LoadShaders(const char* VertexShaderFile, const char* FragmentShaderFile)
{
	// Criar os identificadores de cada um dos shaders
	GLuint VertShaderId = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragShaderId = glCreateShader(GL_FRAGMENT_SHADER);

	std::string VertexShaderSource = ReadFile(VertexShaderFile);
	std::string FragmentShaderSource = ReadFile(FragmentShaderFile);

	assert(!VertexShaderSource.empty());
	assert(!FragmentShaderSource.empty());

	std::cout << "Compilando " << VertexShaderFile << std::endl;
	const char* VertexShaderSourcePtr = VertexShaderSource.c_str();
	glShaderSource(VertShaderId, 1, &VertexShaderSourcePtr, nullptr);
	glCompileShader(VertShaderId);
	CheckShader(VertShaderId);

	std::cout << "Compilando " << FragmentShaderFile << std::endl;
	const char* FragmentShaderSourcePtr = FragmentShaderSource.c_str();
	glShaderSource(FragShaderId, 1, &FragmentShaderSourcePtr, nullptr);
	glCompileShader(FragShaderId);
	CheckShader(FragShaderId);

	std::cout << "Linkando Programa" << std::endl;
	GLuint ProgramId = glCreateProgram();
	glAttachShader(ProgramId, VertShaderId);
	glAttachShader(ProgramId, FragShaderId);
	glLinkProgram(ProgramId);

	// Verificar o programa
	GLint Result = GL_TRUE;
	glGetProgramiv(ProgramId, GL_LINK_STATUS, &Result);

	if (Result == GL_FALSE)
	{
		GLint InfoLogLength = 0;
		glGetProgramiv(ProgramId, GL_INFO_LOG_LENGTH, &InfoLogLength);

		if (InfoLogLength > 0)
		{
			std::string ProgramInfoLog(InfoLogLength, '\0');
			glGetProgramInfoLog(ProgramId, InfoLogLength, nullptr, &ProgramInfoLog[0]);

			std::cout << "Erro ao linkar programa" << std::endl;
			std::cout << ProgramInfoLog << std::endl;

			assert(false);
		}
	}

	glDetachShader(ProgramId, VertShaderId);
	glDetachShader(ProgramId, FragShaderId);

	glDeleteShader(VertShaderId);
	glDeleteShader(FragShaderId);

	return ProgramId;
}

GLuint LoadTexture(const char* TextureFile)
{
	std::cout << "Carregando Textura " << TextureFile << std::endl;

	int TextureWidth = 0;
	int TextureHeight = 0;
	int NumberOfComponents = 0;
	unsigned char* TextureData = stbi_load(TextureFile, &TextureWidth, &TextureHeight, &NumberOfComponents, 3);
	assert(TextureData);

	// Gerar o Identifador da Textura
	GLuint TextureId;
	glGenTextures(1, &TextureId);

	// Habilita a textura para ser modificada
	glBindTexture(GL_TEXTURE_2D, TextureId);

	// Copia a textura para a memória da GPU
	GLint Level = 0;
	GLint Border = 0;
	glTexImage2D(GL_TEXTURE_2D, Level, GL_RGB, TextureWidth, TextureHeight, Border, GL_RGB, GL_UNSIGNED_BYTE, TextureData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(TextureData);
	return TextureId;
}

void MouseButtonCallback(GLFWwindow* Window, int Button, int Action, int Modifiers)
{
	// std::cout << "Button: " << Button << " Action: " << Action << " Modifiers: " << Modifiers << std::endl;

	if (Button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (Action == GLFW_PRESS)
		{
			glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

			double X, Y;
			glfwGetCursorPos(Window, &X, &Y);

			Camera.PreviousCursor = glm::vec2{ X, Y };
			Camera.bEnableMouseMovement = true;
		}
		else if (Action == GLFW_RELEASE)
		{
			glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

			Camera.bEnableMouseMovement = false;
		}
	}
}

void MouseMotionCallback(GLFWwindow* Window, double X, double Y)
{
	// std::cout << "X: " << X << " Y: " << Y << std::endl;
	Camera.MouseMove(X, Y);
}

void KeyCallback(GLFWwindow* Window, int Key, int ScanCode, int Action, int Modifers)
{
	// std::cout << "Key: " << Key << " ScanCode: " << ScanCode << " Action: " << Action << " Modifiers: " << Modifers << std::endl;

	if (Action == GLFW_PRESS)
	{
		switch (Key)
		{
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(Window, true);
				break;

			case GLFW_KEY_W:
				Camera.MoveForward(50.0f);
				break;

			case GLFW_KEY_S:
				Camera.MoveForward(-50.0f);
				break;

			case GLFW_KEY_A:
				Camera.MoveRight(-50.0f);
				break;

			case GLFW_KEY_D:
				Camera.MoveRight(50.0f);
				break;

			default:
				break;
		}
	}
	else if (Action == GLFW_RELEASE)
	{
		switch (Key)
		{
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(Window, true);
				break;

			case GLFW_KEY_W:
				Camera.MoveForward(0.0f);
				break;

			case GLFW_KEY_S:
				Camera.MoveForward(0.0f);
				break;

			case GLFW_KEY_A:
				Camera.MoveRight(0.0f);
				break;

			case GLFW_KEY_D:
				Camera.MoveRight(0.0f);
				break;

			default:
				break;
		}
	}
}

void Resize(GLFWwindow* Window, int NewWidth, int NewHeight) {
	Width = NewWidth;
	Height = NewHeight;

	Camera.AspectRatio = static_cast<float>(Width) / Height;
	glViewport(0, 0, Width, Height);
}

int main()
{
	if (!glfwInit())
	{
		std::cout << "Erro ao inicializar o GLFW" << std::endl;
		return 1;
	}

	glfwWindowHint(GLFW_DEPTH_BITS, 32);

	GLFWwindow* Window = glfwCreateWindow(Width, Height, "Blue Marble", nullptr, nullptr);

	if (!Window)
	{
		std::cout << "Erro ao criar janela" << std::endl;
		glfwTerminate();
		return 1;
	}

	glfwSetMouseButtonCallback(Window, MouseButtonCallback);
	glfwSetCursorPosCallback(Window, MouseMotionCallback);
	glfwSetKeyCallback(Window, KeyCallback);
	glfwSetFramebufferSizeCallback(Window, Resize);

	glfwMakeContextCurrent(Window);
	glfwSwapInterval(1);

	if (glewInit() != GLEW_OK)
	{
		std::cout << "Erro ao inicializar o GLEW" << std::endl;
		glfwTerminate();
		return 1;
	}

	GLint GLMajorVersion = 0;
	GLint GLMinorVersion = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &GLMajorVersion);
	glGetIntegerv(GL_MINOR_VERSION, &GLMinorVersion);
	std::cout << "OpenGL Version  : " << GLMajorVersion << "." << GLMinorVersion << std::endl;
	std::cout << "OpenGL Vendor   : " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "OpenGL Renderer : " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "OpenGL Version  : " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL Version    : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	// Habilita o Buffer de Profundidade
	glEnable(GL_DEPTH_TEST);

	// Escolhe a função de teste de profundidade.
	glDepthFunc(GL_ALWAYS);

	glDisable(GL_CULL_FACE);
	glEnable(GL_CULL_FACE);

	// Compilar o vertex e o fragment shader
	GLuint ProgramId = LoadShaders("shaders/triangle_vert.glsl", "shaders/triangle_frag.glsl");

	// Gera a Geometria da esfera e copia os dados para a GPU 
	std::vector<Vertex> SphereVertices;
	std::vector<Triangle> SphereIndices;
	GenerateSphere(100, SphereVertices, SphereIndices);
	GLuint SphereVertexBuffer, SphereElementBuffer;
	glGenBuffers(1, &SphereVertexBuffer);
	glGenBuffers(1, &SphereElementBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, SphereVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, SphereVertices.size() * sizeof(Vertex), SphereVertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SphereElementBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, SphereIndices.size() * sizeof(Triangle), SphereIndices.data(), GL_STATIC_DRAW);

	// Criar uma fonte de luz direcional
	DirectionalLight Light;
	Light.Direction = glm::vec3(0.0f, 0.0f, -1.5f);
	
	
	

	// Carregar a Textura para a Memoria de Vídeo
	GLuint EarthTextureId = LoadTexture("textures/terra.jpg");
	GLuint MercuryTextureId = LoadTexture("textures/mercurio.jpg");
	GLuint VenusTextureId = LoadTexture("textures/venus.jpg");
	GLuint MarsTextureId = LoadTexture("textures/marte.jpg");
	GLuint JupterTextureId = LoadTexture("textures/jupiter.jpg");
	GLuint SaturnTextureId = LoadTexture("textures/saturno.jpg");
	GLuint UranusTextureId = LoadTexture("textures/urano.jpg");
	GLuint NeptuneTextureId = LoadTexture("textures/netuno.jpg");
	GLuint SunTextureId = LoadTexture("textures/sol.jpg");
	GLuint MoonTextureId = LoadTexture("textures/lua.jpg");
	GLuint EarthCloudsTextureId = LoadTexture("textures/terra_nuvens.jpg");
	GLuint VenusCloudsTextureId = LoadTexture("textures/venus_nuvens.jpg");

	// Configura a cor de fundo
	glClearColor(0.0f, 0.0f, 0.0f, 1.0);

	// Identificador do Vertex Array Object (VAO)
	GLuint SphereVAO;

	// Gerar o identificador do VAO
	glGenVertexArrays(1, &SphereVAO);

	// Habilitar o VAO
	glBindVertexArray(SphereVAO);

	// Habilita o atributo na posição 0
	// Esse vai ser o identificador que vamos usar no shader para ler a posiçãoo de cada vértice.
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	// Diz para o OpenGL que o VertexBuffer vai ficar associado ao atributo 0
	// glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, SphereVertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SphereElementBuffer);

	// Informa ao OpenGL onde, dentro do VertexBuffer, os vértices estão.
	// No caso o array Triangles é tudo o que a gente precisa
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, Normal)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, Color)));
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, UV)));

	// Disabilitar o VAO
	glBindVertexArray(0);

	double PreviousTime = glfwGetTime();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	while (!glfwWindowShouldClose(Window))
	{
		double CurrentTime = glfwGetTime();
		double DeltaTime = CurrentTime - PreviousTime;
		if (DeltaTime > 0.0)
		{
			Camera.Update(static_cast<float>(DeltaTime));
			PreviousTime = CurrentTime;
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(ProgramId);

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~TERRA~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

		glm::mat4 ViewMatrix = Camera.GetView();
		glm::mat4 ModelMatrixEarth = glm::rotate(glm::identity<glm::mat4>(), glm::radians(90.0f), glm::vec3{ 1.0f, 0.0f, 0.0f });
		ModelMatrixEarth = glm::translate(ModelMatrixEarth, glm::vec3(sin((float)CurrentTime) * 60.0, 0.0f, cos((float)CurrentTime) * 60.0));
		ModelMatrixEarth = glm::scale(ModelMatrixEarth, glm::vec3(3.0f, 3.0f, 3.0f));
		glm::mat4 NormalMatrix = glm::transpose(glm::inverse(ViewMatrix * ModelMatrixEarth));
		glm::mat4 ModelViewMatrix = ViewMatrix * ModelMatrixEarth;
		glm::mat4 ModelViewProjectionMatrix = Camera.GetViewProjection() * ModelMatrixEarth;

		GLint TimeLoc = glGetUniformLocation(ProgramId, "Time");
		glUniform1f(TimeLoc, CurrentTime);

		GLint NormalMatrixLoc = glGetUniformLocation(ProgramId, "NormalMatrix");
		glUniformMatrix4fv(NormalMatrixLoc, 1, GL_FALSE, glm::value_ptr(NormalMatrix));

		GLint ModelViewMatrixLoc = glGetUniformLocation(ProgramId, "ModelViewMatrix");
		glUniformMatrix4fv(ModelViewMatrixLoc, 1, GL_FALSE, glm::value_ptr(ModelViewMatrix));

		GLint ModelViewProjectionLoc = glGetUniformLocation(ProgramId, "ModelViewProjection");
		glUniformMatrix4fv(ModelViewProjectionLoc, 1, GL_FALSE, glm::value_ptr(ModelViewProjectionMatrix));

		GLint LightIntensityLoc = glGetUniformLocation(ProgramId, "LightIntensity");

		glm::vec4 LightDirectionViewSpace = ViewMatrix * glm::vec4{ Light.Direction, 0.0f };

		GLint LightDirectionLoc = glGetUniformLocation(ProgramId, "LightDirection");
		glUniform3fv(LightDirectionLoc, 1, glm::value_ptr(LightDirectionViewSpace));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, EarthTextureId);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, EarthCloudsTextureId);

		GLint TextureSamplerLoc = glGetUniformLocation(ProgramId, "Texture");
		glUniform1i(TextureSamplerLoc, 0);

		GLint CloudsTextureSamplerLoc = glGetUniformLocation(ProgramId, "CloudsTexture");
		glUniform1i(CloudsTextureSamplerLoc, 1);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBindVertexArray(SphereVAO);
		glDrawElements(GL_TRIANGLES, SphereIndices.size() * 3, GL_UNSIGNED_INT, nullptr);

		glBindTexture(GL_TEXTURE_2D, 0);

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~MERCURIO~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

		
		ViewMatrix = Camera.GetView();
		glm::mat4 ModelMatrixMercury = glm::rotate(glm::identity<glm::mat4>(), glm::radians(90.0f), glm::vec3{ 1.0f, 0.0f, 0.0f });
		ModelMatrixMercury = glm::translate(ModelMatrixMercury, glm::vec3(sin((float)CurrentTime * 0.5) * 20.0, 0.0f, cos((float)CurrentTime * 0.5) * 20.0));
		ModelMatrixMercury = glm::scale(ModelMatrixMercury, glm::vec3(2.0f, 2.0f, 2.0f));
		NormalMatrix = glm::transpose(glm::inverse(ViewMatrix * ModelMatrixMercury));
		ModelViewMatrix = ViewMatrix * ModelMatrixMercury;
		ModelViewProjectionMatrix = Camera.GetViewProjection() * ModelMatrixMercury;

		TimeLoc = glGetUniformLocation(ProgramId, "Time");
		glUniform1f(TimeLoc, CurrentTime);

		NormalMatrixLoc = glGetUniformLocation(ProgramId, "NormalMatrix");
		glUniformMatrix4fv(NormalMatrixLoc, 1, GL_FALSE, glm::value_ptr(NormalMatrix));

		ModelViewMatrixLoc = glGetUniformLocation(ProgramId, "ModelViewMatrix");
		glUniformMatrix4fv(ModelViewMatrixLoc, 1, GL_FALSE, glm::value_ptr(ModelViewMatrix));

		ModelViewProjectionLoc = glGetUniformLocation(ProgramId, "ModelViewProjection");
		glUniformMatrix4fv(ModelViewProjectionLoc, 1, GL_FALSE, glm::value_ptr(ModelViewProjectionMatrix));

		LightIntensityLoc = glGetUniformLocation(ProgramId, "LightIntensity");

		LightDirectionViewSpace = ViewMatrix * glm::vec4{ Light.Direction, 0.0f };

		LightDirectionLoc = glGetUniformLocation(ProgramId, "LightDirection");
		glUniform3fv(LightDirectionLoc, 1, glm::value_ptr(LightDirectionViewSpace));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, MercuryTextureId);

		TextureSamplerLoc = glGetUniformLocation(ProgramId, "Texture");
		glUniform1i(TextureSamplerLoc, 0);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBindVertexArray(SphereVAO);
		glDrawElements(GL_TRIANGLES, SphereIndices.size() * 3, GL_UNSIGNED_INT, nullptr);

		glBindTexture(GL_TEXTURE_2D, 0);

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~VENUS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
		
		
		
		ViewMatrix = Camera.GetView();
		glm::mat4 ModelMatrixVenus = glm::rotate(glm::identity<glm::mat4>(), glm::radians(90.0f), glm::vec3{ 1.0f, 0.0f, 0.0f });
		ModelMatrixVenus = glm::translate(ModelMatrixVenus, glm::vec3(sin((float)CurrentTime * 0.1) * 40.0, 0.0f, cos((float)CurrentTime * 0.1) * 40.0));
		ModelMatrixVenus = glm::scale(ModelMatrixVenus, glm::vec3(3.0f, 3.0f, 3.0f));
		NormalMatrix = glm::transpose(glm::inverse(ViewMatrix * ModelMatrixVenus));
		ModelViewMatrix = ViewMatrix * ModelMatrixVenus;
		ModelViewProjectionMatrix = Camera.GetViewProjection() * ModelMatrixVenus;

		TimeLoc = glGetUniformLocation(ProgramId, "Time");
		glUniform1f(TimeLoc, CurrentTime);

		NormalMatrixLoc = glGetUniformLocation(ProgramId, "NormalMatrix");
		glUniformMatrix4fv(NormalMatrixLoc, 1, GL_FALSE, glm::value_ptr(NormalMatrix));

		ModelViewMatrixLoc = glGetUniformLocation(ProgramId, "ModelViewMatrix");
		glUniformMatrix4fv(ModelViewMatrixLoc, 1, GL_FALSE, glm::value_ptr(ModelMatrixVenus));

		ModelViewProjectionLoc = glGetUniformLocation(ProgramId, "ModelViewProjection");
		glUniformMatrix4fv(ModelViewProjectionLoc, 1, GL_FALSE, glm::value_ptr(ModelViewProjectionMatrix));

		LightIntensityLoc = glGetUniformLocation(ProgramId, "LightIntensity");

		LightDirectionViewSpace = ViewMatrix * glm::vec4{ Light.Direction, 0.0f };

		LightDirectionLoc = glGetUniformLocation(ProgramId, "LightDirection");
		glUniform3fv(LightDirectionLoc, 1, glm::value_ptr(LightDirectionViewSpace));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, VenusTextureId);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, VenusCloudsTextureId);

		TextureSamplerLoc = glGetUniformLocation(ProgramId, "Texture");
		glUniform1i(TextureSamplerLoc, 0);

		CloudsTextureSamplerLoc = glGetUniformLocation(ProgramId, "CloudsTexture");
		glUniform1i(CloudsTextureSamplerLoc, 1);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBindVertexArray(SphereVAO);
		glDrawElements(GL_TRIANGLES, SphereIndices.size() * 3, GL_UNSIGNED_INT, nullptr);

		glBindTexture(GL_TEXTURE_2D, 0);

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~MARTE~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
		
		

		ViewMatrix = Camera.GetView();
		glm::mat4 ModelMatrixMars = glm::rotate(glm::identity<glm::mat4>(), glm::radians(90.0f), glm::vec3{ 1.0f, 0.0f, 0.0f });
		ModelMatrixMars = glm::translate(ModelMatrixMars, glm::vec3(sin((float)CurrentTime * 1.2) * 80.0, 0.0f, cos((float)CurrentTime * 1.2) * 80.0));
		ModelMatrixMars = glm::scale(ModelMatrixMars, glm::vec3(2.0f, 2.0f, 2.0f));
		NormalMatrix = glm::transpose(glm::inverse(ViewMatrix * ModelMatrixMars));
		ModelViewMatrix = ViewMatrix * ModelMatrixMars;
		ModelViewProjectionMatrix = Camera.GetViewProjection() * ModelMatrixMars;

		TimeLoc = glGetUniformLocation(ProgramId, "Time");
		glUniform1f(TimeLoc, CurrentTime);

		NormalMatrixLoc = glGetUniformLocation(ProgramId, "NormalMatrix");
		glUniformMatrix4fv(NormalMatrixLoc, 1, GL_FALSE, glm::value_ptr(NormalMatrix));

		ModelViewMatrixLoc = glGetUniformLocation(ProgramId, "ModelViewMatrix");
		glUniformMatrix4fv(ModelViewMatrixLoc, 1, GL_FALSE, glm::value_ptr(ModelMatrixMars));

		ModelViewProjectionLoc = glGetUniformLocation(ProgramId, "ModelViewProjection");
		glUniformMatrix4fv(ModelViewProjectionLoc, 1, GL_FALSE, glm::value_ptr(ModelViewProjectionMatrix));

		LightIntensityLoc = glGetUniformLocation(ProgramId, "LightIntensity");

		LightDirectionViewSpace = ViewMatrix * glm::vec4{ Light.Direction, 0.0f };

		LightDirectionLoc = glGetUniformLocation(ProgramId, "LightDirection");
		glUniform3fv(LightDirectionLoc, 1, glm::value_ptr(LightDirectionViewSpace));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, MarsTextureId);

		TextureSamplerLoc = glGetUniformLocation(ProgramId, "Texture");
		glUniform1i(TextureSamplerLoc, 0);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBindVertexArray(SphereVAO);
		glDrawElements(GL_TRIANGLES, SphereIndices.size() * 3, GL_UNSIGNED_INT, nullptr);

		glBindTexture(GL_TEXTURE_2D, 0);


		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~JUPITER~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
		
		
		
		ViewMatrix = Camera.GetView();
		glm::mat4 ModelMatrixJupter = glm::rotate(glm::identity<glm::mat4>(), glm::radians(90.0f), glm::vec3{ 1.0f, 0.0f, 0.0f });
		ModelMatrixJupter = glm::translate(ModelMatrixJupter, glm::vec3(sin((float)CurrentTime * 2.4) * 100.0, 0.0f, cos((float)CurrentTime * 2.4) * 100.0));
		ModelMatrixJupter = glm::scale(ModelMatrixJupter, glm::vec3(5.0f, 5.0f, 5.0f));
		NormalMatrix = glm::transpose(glm::inverse(ViewMatrix * ModelMatrixJupter));
		ModelViewMatrix = ViewMatrix * ModelMatrixJupter;
		ModelViewProjectionMatrix = Camera.GetViewProjection() * ModelMatrixJupter;

		TimeLoc = glGetUniformLocation(ProgramId, "Time");
		glUniform1f(TimeLoc, CurrentTime);

		NormalMatrixLoc = glGetUniformLocation(ProgramId, "NormalMatrix");
		glUniformMatrix4fv(NormalMatrixLoc, 1, GL_FALSE, glm::value_ptr(NormalMatrix));

		ModelViewMatrixLoc = glGetUniformLocation(ProgramId, "ModelViewMatrix");
		glUniformMatrix4fv(ModelViewMatrixLoc, 1, GL_FALSE, glm::value_ptr(ModelMatrixJupter));

		ModelViewProjectionLoc = glGetUniformLocation(ProgramId, "ModelViewProjection");
		glUniformMatrix4fv(ModelViewProjectionLoc, 1, GL_FALSE, glm::value_ptr(ModelViewProjectionMatrix));

		LightIntensityLoc = glGetUniformLocation(ProgramId, "LightIntensity");

		LightDirectionViewSpace = ViewMatrix * glm::vec4{ Light.Direction, 0.0f };

		LightDirectionLoc = glGetUniformLocation(ProgramId, "LightDirection");
		glUniform3fv(LightDirectionLoc, 1, glm::value_ptr(LightDirectionViewSpace));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, JupterTextureId);

		TextureSamplerLoc = glGetUniformLocation(ProgramId, "Texture");
		glUniform1i(TextureSamplerLoc, 0);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBindVertexArray(SphereVAO);
		glDrawElements(GL_TRIANGLES, SphereIndices.size() * 3, GL_UNSIGNED_INT, nullptr);

		glBindTexture(GL_TEXTURE_2D, 0);

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~SATURNO~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
		
		
		
		ViewMatrix = Camera.GetView();
		glm::mat4 ModelMatrixSaturn = glm::rotate(glm::identity<glm::mat4>(), glm::radians(90.0f), glm::vec3{ 1.0f, 0.0f, 0.0f });
		ModelMatrixSaturn = glm::translate(ModelMatrixSaturn, glm::vec3(sin((float)CurrentTime * 2) * 120.0, 0.0f, cos((float)CurrentTime * 2) * 120.0));
		ModelMatrixSaturn = glm::scale(ModelMatrixSaturn, glm::vec3(4.0f, 4.0f, 4.0f));
		NormalMatrix = glm::transpose(glm::inverse(ViewMatrix * ModelMatrixSaturn));
		ModelViewMatrix = ViewMatrix * ModelMatrixSaturn;
		ModelViewProjectionMatrix = Camera.GetViewProjection() * ModelMatrixSaturn;

		TimeLoc = glGetUniformLocation(ProgramId, "Time");
		glUniform1f(TimeLoc, CurrentTime);

		NormalMatrixLoc = glGetUniformLocation(ProgramId, "NormalMatrix");
		glUniformMatrix4fv(NormalMatrixLoc, 1, GL_FALSE, glm::value_ptr(NormalMatrix));

		ModelViewMatrixLoc = glGetUniformLocation(ProgramId, "ModelViewMatrix");
		glUniformMatrix4fv(ModelViewMatrixLoc, 1, GL_FALSE, glm::value_ptr(ModelMatrixSaturn));

		ModelViewProjectionLoc = glGetUniformLocation(ProgramId, "ModelViewProjection");
		glUniformMatrix4fv(ModelViewProjectionLoc, 1, GL_FALSE, glm::value_ptr(ModelViewProjectionMatrix));

		LightIntensityLoc = glGetUniformLocation(ProgramId, "LightIntensity");

		LightDirectionViewSpace = ViewMatrix * glm::vec4{ Light.Direction, 0.0f };

		LightDirectionLoc = glGetUniformLocation(ProgramId, "LightDirection");
		glUniform3fv(LightDirectionLoc, 1, glm::value_ptr(LightDirectionViewSpace));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, SaturnTextureId);

		TextureSamplerLoc = glGetUniformLocation(ProgramId, "Texture");
		glUniform1i(TextureSamplerLoc, 0);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBindVertexArray(SphereVAO);
		glDrawElements(GL_TRIANGLES, SphereIndices.size() * 3, GL_UNSIGNED_INT, nullptr);

		glBindTexture(GL_TEXTURE_2D, 0);

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~URANO~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
		
		
		
		ViewMatrix = Camera.GetView();
		glm::mat4 ModelMatrixUranus = glm::rotate(glm::identity<glm::mat4>(), glm::radians(90.0f), glm::vec3{ 1.0f, 0.0f, 0.0f });
		ModelMatrixUranus = glm::translate(ModelMatrixUranus, glm::vec3(sin((float)CurrentTime * 1.5) * 140.0, 0.0f, cos((float)CurrentTime * 1.5) * 140.0));
		ModelMatrixUranus = glm::scale(ModelMatrixUranus, glm::vec3(2.5f, 2.5f, 2.5f));
		NormalMatrix = glm::transpose(glm::inverse(ViewMatrix * ModelMatrixUranus));
		ModelViewMatrix = ViewMatrix * ModelMatrixUranus;
		ModelViewProjectionMatrix = Camera.GetViewProjection() * ModelMatrixUranus;

		TimeLoc = glGetUniformLocation(ProgramId, "Time");
		glUniform1f(TimeLoc, CurrentTime);

		NormalMatrixLoc = glGetUniformLocation(ProgramId, "NormalMatrix");
		glUniformMatrix4fv(NormalMatrixLoc, 1, GL_FALSE, glm::value_ptr(NormalMatrix));

		ModelViewMatrixLoc = glGetUniformLocation(ProgramId, "ModelViewMatrix");
		glUniformMatrix4fv(ModelViewMatrixLoc, 1, GL_FALSE, glm::value_ptr(ModelMatrixUranus));

		ModelViewProjectionLoc = glGetUniformLocation(ProgramId, "ModelViewProjection");
		glUniformMatrix4fv(ModelViewProjectionLoc, 1, GL_FALSE, glm::value_ptr(ModelViewProjectionMatrix));

		LightIntensityLoc = glGetUniformLocation(ProgramId, "LightIntensity");

		LightDirectionViewSpace = ViewMatrix * glm::vec4{ Light.Direction, 0.0f };

		LightDirectionLoc = glGetUniformLocation(ProgramId, "LightDirection");
		glUniform3fv(LightDirectionLoc, 1, glm::value_ptr(LightDirectionViewSpace));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, UranusTextureId);

		TextureSamplerLoc = glGetUniformLocation(ProgramId, "Texture");
		glUniform1i(TextureSamplerLoc, 0);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBindVertexArray(SphereVAO);
		glDrawElements(GL_TRIANGLES, SphereIndices.size() * 3, GL_UNSIGNED_INT, nullptr);

		glBindTexture(GL_TEXTURE_2D, 0);

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~NETUNO~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
		
		
		
		ViewMatrix = Camera.GetView();
		glm::mat4 ModelMatrixNeptune = glm::rotate(glm::identity<glm::mat4>(), glm::radians(90.0f), glm::vec3{ 1.0f, 0.0f, 0.0f });
		ModelMatrixNeptune = glm::translate(ModelMatrixNeptune, glm::vec3(sin((float)CurrentTime * 1.7) * 160.0, 0.0f, cos((float)CurrentTime * 1.7) * 160.0));
		ModelMatrixNeptune = glm::scale(ModelMatrixNeptune, glm::vec3(3.0f, 3.0f, 3.0f));
		NormalMatrix = glm::transpose(glm::inverse(ViewMatrix * ModelMatrixNeptune));
		ModelViewMatrix = ViewMatrix * ModelMatrixNeptune;
		ModelViewProjectionMatrix = Camera.GetViewProjection() * ModelMatrixNeptune;

		TimeLoc = glGetUniformLocation(ProgramId, "Time");
		glUniform1f(TimeLoc, CurrentTime);

		NormalMatrixLoc = glGetUniformLocation(ProgramId, "NormalMatrix");
		glUniformMatrix4fv(NormalMatrixLoc, 1, GL_FALSE, glm::value_ptr(NormalMatrix));

		ModelViewMatrixLoc = glGetUniformLocation(ProgramId, "ModelViewMatrix");
		glUniformMatrix4fv(ModelViewMatrixLoc, 1, GL_FALSE, glm::value_ptr(ModelMatrixNeptune));

		ModelViewProjectionLoc = glGetUniformLocation(ProgramId, "ModelViewProjection");
		glUniformMatrix4fv(ModelViewProjectionLoc, 1, GL_FALSE, glm::value_ptr(ModelViewProjectionMatrix));

		LightIntensityLoc = glGetUniformLocation(ProgramId, "LightIntensity");

		LightDirectionViewSpace = ViewMatrix * glm::vec4{ Light.Direction, 0.0f };

		LightDirectionLoc = glGetUniformLocation(ProgramId, "LightDirection");
		glUniform3fv(LightDirectionLoc, 1, glm::value_ptr(LightDirectionViewSpace));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, NeptuneTextureId);

		TextureSamplerLoc = glGetUniformLocation(ProgramId, "Texture");
		glUniform1i(TextureSamplerLoc, 0);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBindVertexArray(SphereVAO);
		glDrawElements(GL_TRIANGLES, SphereIndices.size() * 3, GL_UNSIGNED_INT, nullptr);

		glBindTexture(GL_TEXTURE_2D, 0);

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~SOL~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
		
		
		
		ViewMatrix = Camera.GetView();
		glm::mat4 ModelMatrixSun = glm::rotate(glm::identity<glm::mat4>(), glm::radians(90.0f), glm::vec3{ 1.0f, 0.0f, 0.0f });
		ModelMatrixSun = glm::scale(ModelMatrixSun, glm::vec3(8.0f, 8.0f, 8.0f));
		NormalMatrix = glm::transpose(glm::inverse(ViewMatrix * ModelMatrixSun));
		ModelViewMatrix = ViewMatrix * ModelMatrixSun;
		ModelViewProjectionMatrix = Camera.GetViewProjection() * ModelMatrixSun;

		TimeLoc = glGetUniformLocation(ProgramId, "Time");
		glUniform1f(TimeLoc, CurrentTime);

		NormalMatrixLoc = glGetUniformLocation(ProgramId, "NormalMatrix");
		glUniformMatrix4fv(NormalMatrixLoc, 1, GL_FALSE, glm::value_ptr(NormalMatrix));

		ModelViewMatrixLoc = glGetUniformLocation(ProgramId, "ModelViewMatrix");
		glUniformMatrix4fv(ModelViewMatrixLoc, 1, GL_FALSE, glm::value_ptr(ModelMatrixSun));

		ModelViewProjectionLoc = glGetUniformLocation(ProgramId, "ModelViewProjection");
		glUniformMatrix4fv(ModelViewProjectionLoc, 1, GL_FALSE, glm::value_ptr(ModelViewProjectionMatrix));

		LightIntensityLoc = glGetUniformLocation(ProgramId, "LightIntensity");

		LightDirectionViewSpace = ViewMatrix * glm::vec4{ Light.Direction, 0.0f };

		LightDirectionLoc = glGetUniformLocation(ProgramId, "LightDirection");
		glUniform3fv(LightDirectionLoc, 1, glm::value_ptr(LightDirectionViewSpace));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, SunTextureId);

		TextureSamplerLoc = glGetUniformLocation(ProgramId, "Texture");
		glUniform1i(TextureSamplerLoc, 0);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBindVertexArray(SphereVAO);
		glDrawElements(GL_TRIANGLES, SphereIndices.size() * 3, GL_UNSIGNED_INT, nullptr);

		glBindTexture(GL_TEXTURE_2D, 0);

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~LUA~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
		
		
		
		ViewMatrix = Camera.GetView();
		glm::mat4 ModelMatrixMoon = glm::rotate(glm::identity<glm::mat4>(), glm::radians(90.0f), glm::vec3{ 1.0f, 0.0f, 0.0f });
		ModelMatrixMoon = glm::translate(ModelMatrixMoon, glm::vec3(sin((float)CurrentTime) * 60.0, 5.0f, cos((float)CurrentTime) * 50.0));
		NormalMatrix = glm::transpose(glm::inverse(ViewMatrix * ModelMatrixMoon));
		ModelViewMatrix = ViewMatrix * ModelMatrixMoon;
		ModelViewProjectionMatrix = Camera.GetViewProjection() * ModelMatrixMoon;

		TimeLoc = glGetUniformLocation(ProgramId, "Time");
		glUniform1f(TimeLoc, CurrentTime);

		NormalMatrixLoc = glGetUniformLocation(ProgramId, "NormalMatrix");
		glUniformMatrix4fv(NormalMatrixLoc, 1, GL_FALSE, glm::value_ptr(NormalMatrix));

		ModelViewMatrixLoc = glGetUniformLocation(ProgramId, "ModelViewMatrix");
		glUniformMatrix4fv(ModelViewMatrixLoc, 1, GL_FALSE, glm::value_ptr(ModelViewMatrix));

		ModelViewProjectionLoc = glGetUniformLocation(ProgramId, "ModelViewProjection");
		glUniformMatrix4fv(ModelViewProjectionLoc, 1, GL_FALSE, glm::value_ptr(ModelViewProjectionMatrix));

		LightIntensityLoc = glGetUniformLocation(ProgramId, "LightIntensity");

		LightDirectionViewSpace = ViewMatrix * glm::vec4{ Light.Direction, 0.0f };

		LightDirectionLoc = glGetUniformLocation(ProgramId, "LightDirection");
		glUniform3fv(LightDirectionLoc, 1, glm::value_ptr(LightDirectionViewSpace));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, MoonTextureId);

		TextureSamplerLoc = glGetUniformLocation(ProgramId, "Texture");
		glUniform1i(TextureSamplerLoc, 0);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBindVertexArray(SphereVAO);
		glDrawElements(GL_TRIANGLES, SphereIndices.size() * 3, GL_UNSIGNED_INT, nullptr);

		glBindTexture(GL_TEXTURE_2D, 0);

		glBindVertexArray(0);

		glBindVertexArray(0);

		glfwPollEvents();
		glfwSwapBuffers(Window);
	}

	//Teste de Luz
	light_setup(); //light environment


	void light_setup(void)
	{ 
		GLfloat ambient[] = { 0.0, 0.0, 0.0, 1.0 };
		GLfloat diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
		GLfloat position[] = { 0.0, 5.0, -5.0, 25.0 };
		glEnable(GL_LIGHTING);

		glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		glEnable(GL_LIGHT0);

		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glEnable(GL_DEPTH_TEST);
	}

	void initLighting()
	{
   		//glMaterialfv(GL_FRONT,GL_AMBIENT,yellow);
    		//glMaterialfv(GL_FRONT,GL_SPECULAR,yellow);
   		glEnable(GL_LIGHTING);
    		glEnable(GL_LIGHT7);

    		glLightfv(GL_LIGHT7, GL_AMBIENT, qAmb);
    		glLightfv(GL_LIGHT7, GL_DIFFUSE, qDif);
    		glLightfv(GL_LIGHT7, GL_SPECULAR, qSpec);
    		//glMaterialfv(GL_FRONT,GL_DIFFUSE,yellow);
	}

	glDeleteBuffers(1, &SphereElementBuffer);
	glDeleteBuffers(1, &SphereVertexBuffer);
	glDeleteVertexArrays(1, &SphereVAO);
	glDeleteProgram(ProgramId);
	glDeleteTextures(1, &EarthTextureId);

	glfwDestroyWindow(Window);
	glfwTerminate();

	return 0;
}

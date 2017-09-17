#include "load_save_png.hpp"
#include "GL.hpp"

#include <SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <chrono>
#include <iostream>
#include <stdexcept>

static GLuint compile_shader(GLenum type, std::string const &source);
static GLuint link_program(GLuint vertex_shader, GLuint fragment_shader);

int main(int argc, char **argv) {
	//Configuration:
	struct {
		std::string title = "Game1: Text/Tiles";
		glm::uvec2 size = glm::uvec2(480, 480);
	} config;

	//------------  initialization ------------

	//Initialize SDL library:
	SDL_Init(SDL_INIT_VIDEO);

	//Ask for an OpenGL context version 3.3, core profile, enable debug:
	SDL_GL_ResetAttributes();
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	//create window:
	SDL_Window *window = SDL_CreateWindow(
		config.title.c_str(),
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		config.size.x, config.size.y,
		SDL_WINDOW_OPENGL /*| SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI*/
	);

	if (!window) {
		std::cerr << "Error creating SDL window: " << SDL_GetError() << std::endl;
		return 1;
	}

	//Create OpenGL context:
	SDL_GLContext context = SDL_GL_CreateContext(window);

	if (!context) {
		SDL_DestroyWindow(window);
		std::cerr << "Error creating OpenGL context: " << SDL_GetError() << std::endl;
		return 1;
	}

	#ifdef _WIN32
	//On windows, load OpenGL extensions:
	if (!init_gl_shims()) {
		std::cerr << "ERROR: failed to initialize shims." << std::endl;
		return 1;
	}
	#endif

	//Set VSYNC + Late Swap (prevents crazy FPS):
	if (SDL_GL_SetSwapInterval(-1) != 0) {
		std::cerr << "NOTE: couldn't set vsync + late swap tearing (" << SDL_GetError() << ")." << std::endl;
		if (SDL_GL_SetSwapInterval(1) != 0) {
			std::cerr << "NOTE: couldn't set vsync (" << SDL_GetError() << ")." << std::endl;
		}
	}

	//Hide mouse cursor (note: showing can be useful for debugging):
	SDL_ShowCursor(SDL_DISABLE);

	//------------ opengl objects / game assets ------------

	//texture:
	GLuint tex = 0;
	glm::uvec2 tex_size = glm::uvec2(0,0);

	{ //load texture 'tex':
		std::vector< uint32_t > data;
		if (!load_png("background.png", &tex_size.x, &tex_size.y, &data, LowerLeftOrigin)) {
			std::cerr << "Failed to load texture." << std::endl;
			exit(1);
		}
		//create a texture object:
		glGenTextures(1, &tex);
		//bind texture object to GL_TEXTURE_2D:
		glBindTexture(GL_TEXTURE_2D, tex);
		//upload texture data from data:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_size.x, tex_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
		//set texture sampling parameters:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	GLuint tex2 = 0;
	glm::uvec2 tex2_size = glm::uvec2(0,0);

	{ //load texture 'tex2':
		std::vector< uint32_t > data;
		if (!load_png("char.png", &tex2_size.x, &tex2_size.y, &data, LowerLeftOrigin)) {
			std::cerr << "Failed to load texture." << std::endl;
			exit(1);
		}
		//create a texture object:
		glGenTextures(1, &tex2);
		//bind texture object to GL_TEXTURE_2D:
		glBindTexture(GL_TEXTURE_2D, tex2);
		//upload texture data from data:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex2_size.x, tex2_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
		//set texture sampling parameters:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	GLuint tex3 = 0;
	glm::uvec2 tex3_size = glm::uvec2(0,0);

	{ //load texture 'tex2':
		std::vector< uint32_t > data;
		if (!load_png("find_message.png", &tex3_size.x, &tex3_size.y, &data, LowerLeftOrigin)) {
			std::cerr << "Failed to load texture." << std::endl;
			exit(1);
		}
		//create a texture object:
		glGenTextures(1, &tex3);
		//bind texture object to GL_TEXTURE_2D:
		glBindTexture(GL_TEXTURE_2D, tex3);
		//upload texture data from data:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex3_size.x, tex3_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
		//set texture sampling parameters:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	GLuint tex4 = 0;
	glm::uvec2 tex4_size = glm::uvec2(0,0);

	{ //load texture 'tex2':
		std::vector< uint32_t > data;
		if (!load_png("mine_message.png", &tex4_size.x, &tex4_size.y, &data, LowerLeftOrigin)) {
			std::cerr << "Failed to load texture." << std::endl;
			exit(1);
		}
		//create a texture object:
		glGenTextures(1, &tex4);
		//bind texture object to GL_TEXTURE_2D:
		glBindTexture(GL_TEXTURE_2D, tex4);
		//upload texture data from data:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex4_size.x, tex4_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
		//set texture sampling parameters:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	GLuint tex5 = 0;
	glm::uvec2 tex5_size = glm::uvec2(0,0);

	{ //load texture 'tex2':
		std::vector< uint32_t > data;
		if (!load_png("found_message.png", &tex5_size.x, &tex5_size.y, &data, LowerLeftOrigin)) {
			std::cerr << "Failed to load texture." << std::endl;
			exit(1);
		}
		//create a texture object:
		glGenTextures(1, &tex5);
		//bind texture object to GL_TEXTURE_2D:
		glBindTexture(GL_TEXTURE_2D, tex5);
		//upload texture data from data:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex5_size.x, tex5_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
		//set texture sampling parameters:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	GLuint tex6 = 0;
	glm::uvec2 tex6_size = glm::uvec2(0,0);

	{ //load texture 'tex2':
		std::vector< uint32_t > data;
		if (!load_png("black_cover.png", &tex6_size.x, &tex6_size.y, &data, LowerLeftOrigin)) {
			std::cerr << "Failed to load texture." << std::endl;
			exit(1);
		}
		//create a texture object:
		glGenTextures(1, &tex6);
		//bind texture object to GL_TEXTURE_2D:
		glBindTexture(GL_TEXTURE_2D, tex6);
		//upload texture data from data:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex6_size.x, tex6_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
		//set texture sampling parameters:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}


	//shader program:
	GLuint program = 0;
	GLuint program_Position = 0;
	GLuint program_TexCoord = 0;
	GLuint program_Color = 0;
	GLuint program_mvp = 0;
	GLuint program_tex = 0;
	{ //compile shader program:
		GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER,
			"#version 330\n"
			"uniform mat4 mvp;\n"
			"in vec4 Position;\n"
			"in vec2 TexCoord;\n"
			"in vec4 Color;\n"
			"out vec2 texCoord;\n"
			"out vec4 color;\n"
			"void main() {\n"
			"	gl_Position = mvp * Position;\n"
			"	color = Color;\n"
			"	texCoord = TexCoord;\n"
			"}\n"
		);

		GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER,
			"#version 330\n"
			"uniform sampler2D tex;\n"
			"in vec4 color;\n"
			"in vec2 texCoord;\n"
			"out vec4 fragColor;\n"
			"void main() {\n"
			"	fragColor = texture(tex, texCoord) * color;\n"
			"}\n"
		);

		program = link_program(fragment_shader, vertex_shader);

		//look up attribute locations:
		program_Position = glGetAttribLocation(program, "Position");
		if (program_Position == -1U) throw std::runtime_error("no attribute named Position");
		program_TexCoord = glGetAttribLocation(program, "TexCoord");
		if (program_TexCoord == -1U) throw std::runtime_error("no attribute named TexCoord");
		program_Color = glGetAttribLocation(program, "Color");
		if (program_Color == -1U) throw std::runtime_error("no attribute named Color");

		//look up uniform locations:
		program_mvp = glGetUniformLocation(program, "mvp");
		if (program_mvp == -1U) throw std::runtime_error("no uniform named mvp");
		program_tex = glGetUniformLocation(program, "tex");
		if (program_tex == -1U) throw std::runtime_error("no uniform named tex");
	}

	//vertex buffer:
	GLuint buffer = 0;
	{ //create vertex buffer
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
	}

	struct Vertex {
		Vertex(glm::vec2 const &Position_, glm::vec2 const &TexCoord_, glm::u8vec4 const &Color_) :
			Position(Position_), TexCoord(TexCoord_), Color(Color_) { }
		glm::vec2 Position;
		glm::vec2 TexCoord;
		glm::u8vec4 Color;
	};
	static_assert(sizeof(Vertex) == 20, "Vertex is nicely packed.");

	//vertex array object:
	GLuint vao = 0;
	{ //create vao and set up binding:
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glVertexAttribPointer(program_Position, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLbyte *)0);
		glVertexAttribPointer(program_TexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLbyte *)0 + sizeof(glm::vec2));
		glVertexAttribPointer(program_Color, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (GLbyte *)0 + sizeof(glm::vec2) + sizeof(glm::vec2));
		glEnableVertexAttribArray(program_Position);
		glEnableVertexAttribArray(program_TexCoord);
		glEnableVertexAttribArray(program_Color);
	}

	//------------ sprite info ------------
	struct SpriteInfo {
		glm::vec2 min_uv = glm::vec2(0.0f);
		glm::vec2 max_uv = glm::vec2(1.0f);
		glm::vec2 rad = glm::vec2(0.5f);
	};


	auto load_sprite = [](std::string const &name) -> SpriteInfo {
		SpriteInfo info;
		//TODO: look up sprite name in table of sprite infos
		return info;
	};


	//------------ game state ------------

	glm::vec2 mouse = glm::vec2(0.0f, 0.0f); //mouse position in [-1,1]x[-1,1] coordinates

	struct {
		glm::vec2 at = glm::vec2(0.0f, 0.0f);
		glm::vec2 radius = glm::vec2(10.0f, 10.0f);
	} camera;
	//correct radius for aspect ratio:
	camera.radius.x = camera.radius.y * (float(config.size.x) / float(config.size.y));

	// list the possible moves for each tile (up, left, down, right)
	int neighbors[30][4] = {{0,0,0,1}, {0,1,1,1}, {0,1,1,1}, {0,1,1,0}, {0,0,1,0},
							{0,0,1,0}, {1,0,0,0}, {1,0,1,1}, {1,1,1,0}, {1,0,1,0},
							{1,0,0,1}, {0,1,0,1}, {1,1,1,0}, {1,0,0,1}, {1,1,0,0},
							{0,0,1,0}, {0,0,1,1}, {1,1,0,1}, {0,1,1,1}, {0,1,1,0},
							{1,0,1,0}, {1,0,1,0}, {0,0,1,0}, {1,0,1,0}, {1,0,0,0},
							{1,0,0,1}, {1,1,0,1}, {1,1,0,1}, {1,1,0,1}, {0,1,0,0}};

	int visited_tiles[30];

	for (int t = 0; t < 30; t++){
		visited_tiles[t] = 0;
	}
	
	int current_row = 3;
	int current_col = 2;
	visited_tiles[current_row * 5 + current_col] = 1;

	float player_x = 0.0;
	float player_y = 0.0;

	bool display_find = false;
	bool display_mine = false;
	bool display_found = false;

	//------------ game loop ------------

	bool should_quit = false;
	while (true) {
		static SDL_Event evt;
		while (SDL_PollEvent(&evt) == 1) {
			//handle input:
			if (evt.type == SDL_MOUSEMOTION) {
				mouse.x = (evt.motion.x + 0.5f) / float(config.size.x) * 2.0f - 1.0f;
				mouse.y = (evt.motion.y + 0.5f) / float(config.size.y) *-2.0f + 1.0f;
			} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
			} else if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_ESCAPE) {
				should_quit = true;
			} else if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_UP) {
				if (neighbors[current_row * 5 + current_col][0] == 1){
					current_row -= 1;
					visited_tiles[current_row * 5 + current_col] = 1;
				}
			} else if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_LEFT) {
				if (neighbors[current_row * 5 + current_col][1] == 1){
					current_col -= 1;
					visited_tiles[current_row * 5 + current_col] = 1;
				}
			} else if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_DOWN) {
				if (neighbors[current_row * 5 + current_col][2] == 1){
					current_row += 1;
					visited_tiles[current_row * 5 + current_col] = 1;
				}
			} else if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_RIGHT) {
				if (neighbors[current_row * 5 + current_col][3] == 1){
					current_col += 1;
					visited_tiles[current_row * 5 + current_col] = 1;
				}
			} else if (evt.type == SDL_QUIT) {
				should_quit = true;
				break;
			}
		}
		if (should_quit) break;

		auto current_time = std::chrono::high_resolution_clock::now();
		static auto previous_time = current_time;
		float elapsed = std::chrono::duration< float >(current_time - previous_time).count();
		previous_time = current_time;

		{ //update game state:
			(void)elapsed;
		}

		//draw output:
		glClearColor(0.5, 0.5, 0.5, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


		{ //draw game state:
			std::vector< Vertex > verts;
			std::vector< Vertex > verts_char;
			std::vector< Vertex > verts_find;
			std::vector< Vertex > verts_mine;
			std::vector< Vertex > verts_found;
			std::vector< Vertex > verts_cover;

			//helper: add rectangle to verts:
			auto rect = [&verts](glm::vec2 const &at, glm::vec2 const &rad, glm::u8vec4 const &tint) {
				verts.emplace_back(at + glm::vec2(-rad.x,-rad.y), glm::vec2(0.0f, 0.0f), tint);
				verts.emplace_back(verts.back());
				verts.emplace_back(at + glm::vec2(-rad.x, rad.y), glm::vec2(0.0f, 1.0f), tint);
				verts.emplace_back(at + glm::vec2( rad.x,-rad.y), glm::vec2(1.0f, 0.0f), tint);
				verts.emplace_back(at + glm::vec2( rad.x, rad.y), glm::vec2(1.0f, 1.0f), tint);
				verts.emplace_back(verts.back());
			};

			//helper: add character to game
			auto character = [&verts_char](glm::vec2 const &at, glm::vec2 const &rad, glm::u8vec4 const &tint) {
				verts_char.emplace_back(at + glm::vec2(-rad.x,-rad.y), glm::vec2(0.0f, 0.0f), tint);
				verts_char.emplace_back(verts_char.back());
				verts_char.emplace_back(at + glm::vec2(-rad.x, rad.y), glm::vec2(0.0f, 1.0f), tint);
				verts_char.emplace_back(at + glm::vec2( rad.x,-rad.y), glm::vec2(1.0f, 0.0f), tint);
				verts_char.emplace_back(at + glm::vec2( rad.x, rad.y), glm::vec2(1.0f, 1.0f), tint);
				verts_char.emplace_back(verts_char.back());
			};

			//helper: add find message to game
			auto find_mess = [&verts_find](glm::vec2 const &at, glm::vec2 const &rad, glm::u8vec4 const &tint) {
				verts_find.emplace_back(at + glm::vec2(-2.5*rad.x,-0.4*rad.y), glm::vec2(0.0f, 0.0f), tint);
				verts_find.emplace_back(verts_find.back());
				verts_find.emplace_back(at + glm::vec2(-2.5*rad.x, 0.4*rad.y), glm::vec2(0.0f, 1.0f), tint);
				verts_find.emplace_back(at + glm::vec2( 2.5*rad.x,-0.4*rad.y), glm::vec2(1.0f, 0.0f), tint);
				verts_find.emplace_back(at + glm::vec2( 2.5*rad.x, 0.4*rad.y), glm::vec2(1.0f, 1.0f), tint);
				verts_find.emplace_back(verts_find.back());
			};

			//helper: add mine message to game
			auto mine_mess = [&verts_mine](glm::vec2 const &at, glm::vec2 const &rad, glm::u8vec4 const &tint) {
				verts_mine.emplace_back(at + glm::vec2(-2.5*rad.x,-0.4*rad.y), glm::vec2(0.0f, 0.0f), tint);
				verts_mine.emplace_back(verts_mine.back());
				verts_mine.emplace_back(at + glm::vec2(-2.5*rad.x, 0.4*rad.y), glm::vec2(0.0f, 1.0f), tint);
				verts_mine.emplace_back(at + glm::vec2( 2.5*rad.x,-0.4*rad.y), glm::vec2(1.0f, 0.0f), tint);
				verts_mine.emplace_back(at + glm::vec2( 2.5*rad.x, 0.4*rad.y), glm::vec2(1.0f, 1.0f), tint);
				verts_mine.emplace_back(verts_mine.back());
			};

			//helper: add found message to game
			auto found_mess = [&verts_found](glm::vec2 const &at, glm::vec2 const &rad, glm::u8vec4 const &tint) {
				verts_found.emplace_back(at + glm::vec2(-2.5*rad.x,-0.4*rad.y), glm::vec2(0.0f, 0.0f), tint);
				verts_found.emplace_back(verts_found.back());
				verts_found.emplace_back(at + glm::vec2(-2.5*rad.x, 0.4*rad.y), glm::vec2(0.0f, 1.0f), tint);
				verts_found.emplace_back(at + glm::vec2( 2.5*rad.x,-0.4*rad.y), glm::vec2(1.0f, 0.0f), tint);
				verts_found.emplace_back(at + glm::vec2( 2.5*rad.x, 0.4*rad.y), glm::vec2(1.0f, 1.0f), tint);
				verts_found.emplace_back(verts_found.back());
			};

			//helper: add cover message to game
			auto cover = [&verts_cover](glm::vec2 const &at, glm::vec2 const &rad, glm::u8vec4 const &tint) {
				verts_cover.emplace_back(at + glm::vec2(-2*rad.x,-1.5*rad.y), glm::vec2(0.0f, 0.0f), tint);
				verts_cover.emplace_back(verts_cover.back());
				verts_cover.emplace_back(at + glm::vec2(-2*rad.x, 1.5*rad.y), glm::vec2(0.0f, 1.0f), tint);
				verts_cover.emplace_back(at + glm::vec2( 2*rad.x,-1.5*rad.y), glm::vec2(1.0f, 0.0f), tint);
				verts_cover.emplace_back(at + glm::vec2( 2*rad.x, 1.5*rad.y), glm::vec2(1.0f, 1.0f), tint);
				verts_cover.emplace_back(verts_cover.back());
			};

			auto draw_sprite = [&verts](SpriteInfo const &sprite, glm::vec2 const &at, float angle = 0.0f) {
				glm::vec2 min_uv = sprite.min_uv;
				glm::vec2 max_uv = sprite.max_uv;
				glm::vec2 rad = sprite.rad;
				glm::u8vec4 tint = glm::u8vec4(0xff, 0xff, 0xff, 0xff);
				glm::vec2 right = glm::vec2(std::cos(angle), std::sin(angle));
				glm::vec2 up = glm::vec2(-right.y, right.x);

				verts.emplace_back(at + right * -rad.x + up * -rad.y, glm::vec2(min_uv.x, min_uv.y), tint);
				verts.emplace_back(verts.back());
				verts.emplace_back(at + right * -rad.x + up * rad.y, glm::vec2(min_uv.x, max_uv.y), tint);
				verts.emplace_back(at + right *  rad.x + up * -rad.y, glm::vec2(max_uv.x, min_uv.y), tint);
				verts.emplace_back(at + right *  rad.x + up *  rad.y, glm::vec2(max_uv.x, max_uv.y), tint);
				verts.emplace_back(verts.back());
			};


			//Draw a sprite "player" at position (5.0, 2.0):
			static SpriteInfo player = load_sprite("player"); //TODO: hoist


			//draw our game ccomponents
			rect(glm::vec2(0.0f, 0.0f), glm::vec2(10.0f), glm::u8vec4(0xff, 0xff, 0xff, 0xff));

			player_x = -8.0f + (current_col * 4.0f);
			player_y = 8.0f - (current_row * 2.5f);
			character(glm::vec2(player_x, player_y), glm::vec2(0.8f), glm::u8vec4(0xff, 0xff, 0xff, 0xff));

			if (current_row == 0 && current_col == 4) {
				display_find = false;
				display_mine = false;
				display_found = true;
			} else if ((current_row == 1 && current_col == 0) || (current_row == 3 && current_col == 0) ||
					   (current_row == 4 && current_col == 2) || (current_row == 5 && current_col == 4)){
				display_find = false;
				display_mine = true;
				display_found = false;
			} else {
				display_find = true;
				display_mine = false;
				display_found = false;
			}

			if (display_find) {
				find_mess(glm::vec2(0.0f, -8.5f), glm::vec2(4.0f), glm::u8vec4(0xff, 0xff, 0xff, 0xff));
			}

			if (display_mine) {
				mine_mess(glm::vec2(0.0f, -8.5f), glm::vec2(4.0f), glm::u8vec4(0xff, 0xff, 0xff, 0xff));
			}
			
			if (display_found){
				found_mess(glm::vec2(0.0f, -8.5f), glm::vec2(4.0f), glm::u8vec4(0xff, 0xff, 0xff, 0xff));
			}
			

			float start_x;
			float start_y;

			for (int row = 0; row < 6; row++){
				for (int col = 0; col < 5; col++){
					if (visited_tiles[(row * 5) + col] == 0){
						start_x = -8.0f + (col * 4.0f);
						start_y = 8.5f - (row * 3.0f);
						cover(glm::vec2(start_x, start_y), glm::vec2(1.0f), glm::u8vec4(0xff, 0xff, 0xff, 0xff));
					}
				}
			}

			//cover(glm::vec2(0.0f, -0.5f), glm::vec2(1.0f), glm::u8vec4(0xff, 0xff, 0xff, 0xff));
			//cover(glm::vec2(0.0f, 8.5f), glm::vec2(1.0f), glm::u8vec4(0xff, 0xff, 0xff, 0xff));

			glBindBuffer(GL_ARRAY_BUFFER, buffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * verts.size(), &verts[0], GL_STREAM_DRAW);

			glUseProgram(program);
			glUniform1i(program_tex, 0);
			glm::vec2 scale = 1.0f / camera.radius;
			glm::vec2 offset = scale * -camera.at;
			glm::mat4 mvp = glm::mat4(
				glm::vec4(scale.x, 0.0f, 0.0f, 0.0f),
				glm::vec4(0.0f, scale.y, 0.0f, 0.0f),
				glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
				glm::vec4(offset.x, offset.y, 0.0f, 1.0f)
			);
			glUniformMatrix4fv(program_mvp, 1, GL_FALSE, glm::value_ptr(mvp));

			glBindTexture(GL_TEXTURE_2D, tex);

			glBindVertexArray(vao);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, verts.size());

			//tex2
			glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * verts_char.size(), &verts_char[0], GL_STREAM_DRAW);

			glBindTexture(GL_TEXTURE_2D, tex2);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, verts_char.size());

			//tex3
			glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * verts_find.size(), &verts_find[0], GL_STREAM_DRAW);

			glBindTexture(GL_TEXTURE_2D, tex3);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, verts_find.size());

			//tex3
			glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * verts_mine.size(), &verts_mine[0], GL_STREAM_DRAW);

			glBindTexture(GL_TEXTURE_2D, tex4);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, verts_mine.size());

			//tex3
			glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * verts_found.size(), &verts_found[0], GL_STREAM_DRAW);

			glBindTexture(GL_TEXTURE_2D, tex5);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, verts_found.size());

			//tex3
			glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * verts_cover.size(), &verts_cover[0], GL_STREAM_DRAW);

			glBindTexture(GL_TEXTURE_2D, tex6);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, verts_cover.size());
		}


		SDL_GL_SwapWindow(window);
	}


	//------------  teardown ------------

	SDL_GL_DeleteContext(context);
	context = 0;

	SDL_DestroyWindow(window);
	window = NULL;

	return 0;
}



static GLuint compile_shader(GLenum type, std::string const &source) {
	GLuint shader = glCreateShader(type);
	GLchar const *str = source.c_str();
	GLint length = source.size();
	glShaderSource(shader, 1, &str, &length);
	glCompileShader(shader);
	GLint compile_status = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
	if (compile_status != GL_TRUE) {
		std::cerr << "Failed to compile shader." << std::endl;
		GLint info_log_length = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);
		std::vector< GLchar > info_log(info_log_length, 0);
		GLsizei length = 0;
		glGetShaderInfoLog(shader, info_log.size(), &length, &info_log[0]);
		std::cerr << "Info log: " << std::string(info_log.begin(), info_log.begin() + length);
		glDeleteShader(shader);
		throw std::runtime_error("Failed to compile shader.");
	}
	return shader;
}

static GLuint link_program(GLuint fragment_shader, GLuint vertex_shader) {
	GLuint program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);
	GLint link_status = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &link_status);
	if (link_status != GL_TRUE) {
		std::cerr << "Failed to link shader program." << std::endl;
		GLint info_log_length = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);
		std::vector< GLchar > info_log(info_log_length, 0);
		GLsizei length = 0;
		glGetProgramInfoLog(program, info_log.size(), &length, &info_log[0]);
		std::cerr << "Info log: " << std::string(info_log.begin(), info_log.begin() + length);
		throw std::runtime_error("Failed to link program");
	}
	return program;
}

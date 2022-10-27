#include <3ds.h>
#include <citro3d.h>
#include <tex3ds.h>
#include <string.h>

#include "vshader_shbin.h"
#include "kitten_t3x.h"
#include "eye_t3x.h"
#include "teapot.h"
#include "graphics.h"
#include "gameObject.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

//using namespace std;

C3D_Tex kitten_tex;
C3D_Tex eyeTexture;
float angleX = 0.0, angleY = 0.0;

Vertex teapot1VertexList[teapot_count / 3];
Vertex teapot2VertexList[teapot_count / 3];

Vertex* sonicVertices;

GameObject teapot1;
GameObject teapot2;


std::string inputfile = "romfs:/sonic.obj";
std::string mtlDir = "romfs:/";
tinyobj::attrib_t attrib;
std::vector<tinyobj::shape_t> shapes;
std::vector<tinyobj::material_t> materials;
std::string warn;
std::string err;

/*tinyobj::ObjReaderConfig reader_config;
tinyobj::ObjReader reader;*/

void sceneInit() {
	romfsInit();

	tinyobj::LoadObj(&attrib, &shapes, &materials, &err, inputfile.c_str(), mtlDir.c_str(), true);

	int s = 3;

	sonicVertices = new Vertex[attrib.vertices.size()];
	int sonicVertexCount = 0;

	// Loop over shapes
	//for (size_t s = 0; s < shapes.size(); s++) {
		// Loop over faces(polygon)
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++) {
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

				float vx = attrib.vertices[3*size_t(idx.vertex_index)+0];
				float vy = attrib.vertices[3*size_t(idx.vertex_index)+1];
				float vz = attrib.vertices[3*size_t(idx.vertex_index)+2];

				float nx = 0;
				float ny = 0;
				float nz = 0;
				float tx = 0;
				float ty = 0;

				// Check if `normal_index` is zero or positive. negative = no normal data
				if (idx.normal_index >= 0) {
					nx = attrib.normals[3*size_t(idx.normal_index)+0];
					ny = attrib.normals[3*size_t(idx.normal_index)+1];
					nz = attrib.normals[3*size_t(idx.normal_index)+2];
				}

				// Check if `texcoord_index` is zero or positive. negative = no texcoord data
				if (idx.texcoord_index >= 0) {
					tx = attrib.texcoords[2*size_t(idx.texcoord_index)+0];
					ty = attrib.texcoords[2*size_t(idx.texcoord_index)+1];
				}

				sonicVertices[sonicVertexCount++] = {{vx, vy, vz}, {tx, ty}, {nx, ny, nz}};
			}
			index_offset += fv;

			// per-face material
			shapes[s].mesh.material_ids[f];
		}
	//}

	initGraphics();

	for(int v = 0; v < teapot_count / 9; v++) {
		float vert0[3] = {teapot[v * 9 + 0], teapot[v * 9 + 1], teapot[v * 9 + 2]};
		float vert1[3] = {teapot[v * 9 + 3], teapot[v * 9 + 4], teapot[v * 9 + 5]};
		float vert2[3] = {teapot[v * 9 + 6], teapot[v * 9 + 7], teapot[v * 9 + 8]};

		float normX = calculateNormalX(vert0, vert1, vert2);
		float normY = calculateNormalY(vert0, vert1, vert2);
		float normZ = calculateNormalZ(vert0, vert1, vert2);

		teapot1VertexList[v * 3 + 0] = {{vert0[0], vert0[1], vert0[2]}, {0.0f, 0.0f}, {normX, normY, normZ}};
		teapot1VertexList[v * 3 + 1] = {{vert1[0], vert1[1], vert1[2]}, {1.0f, 1.0f}, {normX, normY, normZ}};
		teapot1VertexList[v * 3 + 2] = {{vert2[0], vert2[1], vert2[2]}, {0.0f, 1.0f}, {normX, normY, normZ}};

		teapot2VertexList[v * 3 + 0] = {{vert0[0] + 2, vert0[1], vert0[2]}, {0.0f, 0.0f}, {normX, normY, normZ}};
		teapot2VertexList[v * 3 + 1] = {{vert1[0] + 2, vert1[1], vert1[2]}, {1.0f, 1.0f}, {normX, normY, normZ}};
		teapot2VertexList[v * 3 + 2] = {{vert2[0] + 2, vert2[1], vert2[2]}, {0.0f, 1.0f}, {normX, normY, normZ}};
	}

	Mtx_PerspTilt(&projection, C3D_AngleFromDegrees(80.0f), C3D_AspectRatioTop, 0.01f, 1000.0f, false);

	if(!loadTextureFromMem(&kitten_tex, kitten_t3x, kitten_t3x_size)) {svcBreak(USERBREAK_PANIC);}
	C3D_TexSetFilter(&kitten_tex, GPU_LINEAR, GPU_NEAREST);

	if(!loadTextureFromMem(&eyeTexture, eye_t3x, eye_t3x_size)) {svcBreak(USERBREAK_PANIC);}
	C3D_TexSetFilter(&eyeTexture, GPU_LINEAR, GPU_NEAREST);

	teapot1.loadVertices(sonicVertices, sonicVertexCount);
	teapot1.setTextures(&eyeTexture, 1);
	teapot1.initialize(GetVector3(new float[]{0, 0, 0}), GetVector3(new float[]{0, 0, 0}), GetVector3(new float[]{0, 0, 0}));
}

void sceneRender(void) {
	C3D_Mtx modelView;
	Mtx_Identity(&modelView);
	Mtx_Translate(&modelView, 0.0, 0.0, -2.0 + 0.5*sinf(angleX), true);
	Mtx_RotateX(&modelView, angleX, true);
	Mtx_RotateY(&modelView, angleY, true);

	angleX += M_PI / 180;
	angleY += M_PI / 360;

    updateUniforms(&modelView);

	teapot1.updateVertices();
	teapot1.draw();

	/*C3D_SetBufInfo(&buf1Info);
	memcpy(vbo1Data, teapot1VertexList, sizeof(teapot1VertexList));

	C3D_TexBind(0, &kitten_tex);
	C3D_DrawArrays(GPU_TRIANGLES, 0, teapot_count / 3 / 3 * 2);

	C3D_TexBind(0, &eyeTexture);
	C3D_DrawArrays(GPU_TRIANGLES, teapot_count / 3 / 3 * 2, teapot_count / 3 / 3);

	C3D_SetBufInfo(&buf2Info);
	memcpy(vbo2Data, teapot2VertexList, sizeof(teapot2VertexList));

	C3D_TexBind(0, &kitten_tex);
	C3D_DrawArrays(GPU_TRIANGLES, 0, teapot_count / 3 / 3 * 2);

	C3D_TexBind(0, &eyeTexture);
	C3D_DrawArrays(GPU_TRIANGLES, teapot_count / 3 / 3 * 2, teapot_count / 3 / 3);*/
}

void sceneExit() {
	C3D_TexDelete(&kitten_tex);
	C3D_TexDelete(&eyeTexture);

	teapot1.free();
	//linearFree(vbo1Data);
	//linearFree(vbo2Data);

	shaderProgramFree(&shaderProgram);
	DVLB_Free(vshader_dvlb);
}

int main() {
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);

	C3D_RenderTarget* target = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
	C3D_RenderTargetSetOutput(target, GFX_TOP, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);

	sceneInit();

	while (aptMainLoop()) {
		hidScanInput();

		// Respond to user input
		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu

		// Render the scene
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW); // can be 0
			C3D_RenderTargetClear(target, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
			C3D_FrameDrawOn(target);
			sceneRender();
		C3D_FrameEnd(0);
	}

	// Deinitialize the scene
	sceneExit();

	// Deinitialize graphics
	C3D_Fini();
	gfxExit();
	return 0;
}

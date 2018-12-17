#pragma once
#define _XM_NO_INTRINSICS_
#define XM_NO_ALIGNMENT
#include <d3d11.h>
#include <d3dx11.h>
#include <dxerr.h>
#include <xnamath.h>
#include <vector>
using namespace std;

// store a sting with x and y coords (-1.0 to +1.0), and size (0.0+)
struct string_2d
{
	string s;
	float x;
	float y;
	float size;
};

// Define vertex structure
struct POS_TEX_VERTEX
{
	XMFLOAT3 Pos;
	XMFLOAT2 Texture;
};

// increase if more characters requird
const int MAX_CHARACTERS = 10000;


class Text2D
{
private:
	vector<string_2d> s2d;							// stores list of strings with coords

	ID3D11Device*           pD3DDevice;
	ID3D11DeviceContext*    pImmediateContext;

	ID3D11ShaderResourceView*	pTexture;    
	ID3D11SamplerState*			pSampler; 

	POS_TEX_VERTEX vertices[MAX_CHARACTERS * 6];	// system memory vertex list, to be copied to vertex buffer
	
	ID3D11Buffer*			pVertexBuffer;          
	ID3D11VertexShader*		pVShader;         
	ID3D11PixelShader*		pPShader;          
	ID3D11InputLayout*		pInputLayout; 
 
	ID3D11DepthStencilState* pDepthEnabledStencilState;		// state to turn on Z buffer
	ID3D11DepthStencilState* pDepthDisabledStencilState;	// state to turn off Z buffer

public:
	Text2D(string filename, ID3D11Device* device, ID3D11DeviceContext* context); // pass in filename of font, device and context
	~Text2D(void);

	// add a string with position and size to the list
	// positions are from -1.0 to +1.0 for x and y, represents top left of string on screen
	// size is fraction of screen size
	void AddText(string s, float x, float y, float size) ;

	// render all strings at once
	void RenderText();

};
	

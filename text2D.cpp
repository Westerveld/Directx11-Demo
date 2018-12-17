#include "Text2D.h"

// constructor does not handle fails gracefully for simplicity, simply exits
// track errors by looking at debug output or using debugger
// scope for more advanced code to improve error handling
Text2D::Text2D(string filename, ID3D11Device* device, ID3D11DeviceContext* context)
{
	pD3DDevice = device;
	pImmediateContext = context;

	// Set up and create vertex buffer
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;										// Used by CPU and GPU
	bufferDesc.ByteWidth = sizeof(vertices);									// Total size of buffer,
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;							// Use as a vertex buffer
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;							// Allow CPU access
	HRESULT hr = pD3DDevice->CreateBuffer(&bufferDesc, NULL, &pVertexBuffer);	// Create the buffer

	if(FAILED(hr)) exit(0);

	// Load and compile pixel and vertex shaders - use vs_5_0 to target DX11 hardware only
	ID3DBlob *VS, *PS, *error;
	hr = D3DX11CompileFromFile("text2d_shaders.hlsl", 0, 0, "TextVS", "vs_4_0", 0, 0, 0, &VS, &error, 0);

	if(error != 0)
	{
		OutputDebugStringA((char*)error->GetBufferPointer());
		error->Release();
		if(FAILED(hr))exit(0);
	}

	hr = D3DX11CompileFromFile("text2d_shaders.hlsl", 0, 0, "TextPS", "ps_4_0", 0, 0, 0, &PS, &error, 0);

	if(error != 0)
	{
		OutputDebugStringA((char*)error->GetBufferPointer());
		error->Release();
		if(FAILED(hr)) exit(0);
	}

	// Create shader objects
	hr = pD3DDevice->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &pVShader);
	if(FAILED(hr)) exit(0);
	
	hr = pD3DDevice->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &pPShader);
	if(FAILED(hr)) exit(0);

	// Create and set the input layout object
	D3D11_INPUT_ELEMENT_DESC iedesc[] =
	{
		 {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		 {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},

	};

	hr = pD3DDevice->CreateInputLayout(iedesc, ARRAYSIZE(iedesc), VS->GetBufferPointer(), VS->GetBufferSize(), &pInputLayout);
	if(FAILED(hr)) exit(0);

	// Load in the font texture from given filename
	hr = D3DX11CreateShaderResourceViewFromFile(pD3DDevice, filename.c_str(),NULL, NULL, &pTexture,NULL); 
	if(FAILED(hr)) exit(0);
	
	// Create sampler for texture
	D3D11_SAMPLER_DESC sampler_desc;
	ZeroMemory(&sampler_desc, sizeof(sampler_desc));
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

    hr = pD3DDevice->CreateSamplerState(&sampler_desc, &pSampler);

	// Create 2 depth stencil states to turn Z buffer on and off
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	depthStencilDesc.DepthEnable = false;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	hr = device->CreateDepthStencilState(&depthStencilDesc, &pDepthDisabledStencilState);
	if(FAILED(hr)) exit(0);

	depthStencilDesc.DepthEnable = true;
	hr = device->CreateDepthStencilState(&depthStencilDesc, &pDepthEnabledStencilState);
	if(FAILED(hr)) exit(0);
}

// add a string with position and size to the list
// positions are from -1.0 to +1.0 for x and y, represents top left of string on screen
// size is fraction of screen size
void Text2D::AddText(string s, float x, float y, float size)
{ 
	string_2d temp; 
	temp.s = s ;
	temp.x = x; 
	temp.y = y; 
	temp.size = size; 
	s2d.push_back(temp); 
}

// render all text at given positions
// scope for improvement to add alpha blended text
void Text2D::RenderText(void)
{
	int current_char = 0; // keep track of number of characters so far

	// loop through all the added string_2d
	for(unsigned int i=0; i < s2d.size(); i++)
	{
		float tempx = s2d[i].x;
		float tempy = s2d[i].y;
		float temps = s2d[i].size;

		// loop through each character
		for(unsigned int j = 0;j < s2d[i].s.length(); j++)
		{
			// create 6 vertices with given size and xy position
			vertices[current_char*6].Pos.x = tempx;
			vertices[current_char*6].Pos.y = tempy;
			vertices[current_char*6+1].Pos.x = tempx + temps;
			vertices[current_char*6+1].Pos.y = tempy;				
			vertices[current_char*6+2].Pos.x = tempx + temps;
			vertices[current_char*6+2].Pos.y = tempy - temps;			
			vertices[current_char*6+3].Pos.x = tempx;
			vertices[current_char*6+3].Pos.y = tempy;
			vertices[current_char*6+4].Pos.x = tempx + temps;
			vertices[current_char*6+4].Pos.y = tempy - temps;		
			vertices[current_char*6+5].Pos.x = tempx;
			vertices[current_char*6+5].Pos.y = tempy - temps;

			// set all z to 1.0 to avoid being clipped
			vertices[current_char*6].Pos.z = 1.0;
			vertices[current_char*6+1].Pos.z = 1.0;
			vertices[current_char*6+2].Pos.z = 1.0;
			vertices[current_char*6+3].Pos.z = 1.0;
			vertices[current_char*6+4].Pos.z = 1.0;
			vertices[current_char*6+5].Pos.z = 1.0;
		
			// this code assumes 26 characters across and 4 lines in texture, containing lower, upper, number, symbols
			const int NUMLINES = 4;

			char c = s2d[i].s[j]; //  get current character
			
			float texy, texx; // temp tex coords

			// determine texture coord to begin at, based on character
			// 1/26th added to x texture coord for each subsequent letter in alhpabet
			if(c >= 'a' && c <= 'z') // lowercase
			{
				texy= 0.0; // first line
				texx  = (c-'a') * 1.0f / 26.0f; 
			}
			else if(c >= 'A' && c <= 'Z') // uppercase
			{
				texy= 1.0/NUMLINES; //second line
				texx  = (c-'A') *1.0f /26.0f;
			}
			else if(c >= '0' && c <= '9') // numbers
			{
				texy=  2.0/NUMLINES; // third line
				texx  = (c-'0') *1.0f /26.0f;
			}
			else // add any symbol code here
			{
				texx=0;
				texy=  3.0/NUMLINES;
				//symbols to display can go here
			}

			// set correct texture coordinates for letter
			vertices[current_char*6].Texture.x = texx;
			vertices[current_char*6].Texture.y = texy;
			vertices[current_char*6+1].Texture.x = texx + 1.0f/ 26.0f;
			vertices[current_char*6+1].Texture.y = texy;
			vertices[current_char*6+2].Texture.x =texx + 1.0f/ 26.0f;
			vertices[current_char*6+2].Texture.y = texy+ 1.0f/ NUMLINES;
			vertices[current_char*6+3].Texture.x = texx;
			vertices[current_char*6+3].Texture.y = texy;
			vertices[current_char*6+4].Texture.x = texx + 1.0f/ 26.0f;
			vertices[current_char*6+4].Texture.y = texy+ 1.0f/ NUMLINES;
			vertices[current_char*6+5].Texture.x = texx;
			vertices[current_char*6+5].Texture.y = texy+ 1.0f/ NUMLINES;

			current_char++; 
			tempx += temps; // position next character along in x 
		}
	}

	// clear out the vector every frame, otherwise will grow forever
	s2d.clear();

	// Copy the vertices into the buffer
	D3D11_MAPPED_SUBRESOURCE ms;
	pImmediateContext->Map(pVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);	// Lock the buffer to allow writing
	memcpy(ms.pData, vertices, sizeof(vertices[0])*current_char*6);						// Copy the data -  only upload those that are used
	pImmediateContext->Unmap(pVertexBuffer, NULL);	

	// set all rendering states
	pImmediateContext->PSSetSamplers(0, 1, &pSampler);
	pImmediateContext->PSSetShaderResources(0, 1, &pTexture);
	pImmediateContext->VSSetShader(pVShader, 0, 0);
	pImmediateContext->PSSetShader(pPShader, 0, 0);
	pImmediateContext->IASetInputLayout(pInputLayout);

	UINT stride = sizeof(POS_TEX_VERTEX);
	UINT offset = 0;
	pImmediateContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
	pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	// turn off Z buffer so text always on top
	pImmediateContext->OMSetDepthStencilState(pDepthDisabledStencilState, 1);

	// draw all added characters
	pImmediateContext->Draw(current_char*6, 0);

	// turn on Z buffer so other rendering can use it
	pImmediateContext->OMSetDepthStencilState(pDepthEnabledStencilState, 1);
}


Text2D::~Text2D(void)
{
	if(pDepthDisabledStencilState) pDepthDisabledStencilState->Release();
	if(pDepthEnabledStencilState) pDepthEnabledStencilState->Release();
	if(pTexture) pTexture->Release();
	if(pSampler) pSampler->Release();
	if(pVertexBuffer) pVertexBuffer->Release();
	if(pVShader) pVShader->Release();
	if(pPShader) pPShader->Release();
	if(pInputLayout) pInputLayout->Release();	

}

// turn off fopen warnings
#define _CRT_SECURE_NO_WARNINGS

#include "ObjFileModel.h"


// draw object
void ObjFileModel::Draw(void)
{
	UINT stride = sizeof(MODEL_POS_TEX_NORM_VERTEX);
	UINT offset = 0;
	pImmediateContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
	pImmediateContext->Draw(numverts, 0);
}


// load object from obj file in constructor
ObjFileModel::ObjFileModel(char* fname, ID3D11Device* device, ID3D11DeviceContext* context)
{
	pD3DDevice = device;
	pImmediateContext = context;

	if(loadfile(fname)==0)
	{
		// file not loaded, check debug output;
		filename="FILE NOT LOADED";
		return;
	}

	filename = fname;

	parsefile();

	createVB();

	delete[] fbuffer; // delete file buffer created in loadfile()
}


// load wavefront object file. adds terminating \n so last line of file can be correctly parsed as a 'line' later
// basic loader - only deals with vertices v, texcoords vt, normals vn 
//              - only copes with triangular meshes (no quads)
//              - doesn't deal with textures or materials
int ObjFileModel::loadfile(char* fname)
{
	FILE* pFile;

	pFile = fopen(fname , "r"); // if changed to bin format will read carriage return \r (0d) as well as \n (0a) into fbuffer, may need to add \r checks(but seemed to work with basic test)
	if (pFile==NULL) { DXTRACE_MSG("Failed to open model file");DXTRACE_MSG(fname); return 0 ;}

	// get file size
	fseek(pFile, 0, SEEK_END);
	fbuffersize = ftell(pFile);
	rewind(pFile);

	// allocate memory for entire file size
	fbuffer  = new char[fbuffersize+1]; // 1 added to cope with adding a \n later in case file doesn't end with \n 
	if (fbuffer == NULL) {fclose(pFile); DXTRACE_MSG("Failed allocate memory for model file");DXTRACE_MSG(fname); return 0 ;}

	// copy file into memory
	actualsize = fread(fbuffer,1,fbuffersize,pFile); // actualsize may be less than fbuffersize in text mode as \r are stripped
	if (actualsize == 0) {fclose(pFile); DXTRACE_MSG("Failed to read model file");DXTRACE_MSG(fname); return 0 ;}

	// add a newline at end in case file does not, so can deal with whole buffer as a set of lines of text
	fbuffer[actualsize] = '\n'; fclose(pFile);

	return 1;
}
 

// uses concept of getting parsable tokens seperated by whitespace and '/'
// one line of file is parsed at a time, lines seperated by '\n'
void ObjFileModel::parsefile()
{
	tokenptr=0; // token pointer points to first element of buffer

	int tokenstart, tokenlength;

	xyz tempxyz;
	xy tempxy;

	bool success;
	int line=0;

	do
	{	
		line++; // keep track of current line number for error reporting

		if(!getnexttoken(tokenstart, tokenlength)) continue; // get first token on line, go to next line if first token is \n

		// ADD FURTHER KEYWORDS HERE TO EXTEND CAPABILITIES
		if(strncmp(&fbuffer[tokenstart], "v ", 2)==0) // VERTEX POSITION - note the space in the string is needed (see vt, etc)
		{
			success=true; // used to see if correct number of tokens left on line for this type of attribute
			success = success && getnexttoken(tokenstart, tokenlength);
			tempxyz.x = (float) atof(&fbuffer[tokenstart]);
			success = success && getnexttoken(tokenstart, tokenlength);
			tempxyz.y = (float) atof(&fbuffer[tokenstart]);
			success = success && getnexttoken(tokenstart, tokenlength);
			tempxyz.z = (float) atof(&fbuffer[tokenstart]);

			// if not correct number of tokens, display error in debug output
			if(!success) {char s[100] = "ERROR: Badly formatted vertex, line : "; _itoa(line, &s[strlen(s)], 10); strcat(s, " : "); strcat(s, filename.c_str());  DXTRACE_MSG(s); }

			position_list.push_back(tempxyz); // add a new element to the list

		}
		else if(strncmp(&fbuffer[tokenstart], "vt", 2)==0) // TEXTURE COORDINATES
		{
			success=true;
			success = success && getnexttoken(tokenstart, tokenlength);
			tempxy.x = (float) atof(&fbuffer[tokenstart]);
			success = success && getnexttoken(tokenstart, tokenlength);
			tempxy.y = (float) atof(&fbuffer[tokenstart]);

			if(!success) {char s[100] = "ERROR: Badly formatted texture coordinate, line : "; _itoa(line, &s[strlen(s)], 10); strcat(s, " : "); strcat(s, filename.c_str());  DXTRACE_MSG(s); }

			texcoord_list.push_back(tempxy);
		}
		else if(strncmp(&fbuffer[tokenstart], "vn", 2)==0)  // NORMALS
		{
			success=true;
			success = success && getnexttoken(tokenstart, tokenlength);
			tempxyz.x = (float) atof(&fbuffer[tokenstart]);
			success = success && getnexttoken(tokenstart, tokenlength);
			tempxyz.y = (float) atof(&fbuffer[tokenstart]);
			success = success && getnexttoken(tokenstart, tokenlength);
			tempxyz.z = (float) atof(&fbuffer[tokenstart]);

			if(!success) {char s[100] = "ERROR: Badly formatted normal, line : "; _itoa(line, &s[strlen(s)], 10); strcat(s, " : "); strcat(s, filename.c_str());  DXTRACE_MSG(s); }

			normal_list.push_back(tempxyz);
		}
		else if(strncmp(&fbuffer[tokenstart], "f ", 2)==0)  // FACE - only deals with triangles so far
		{
			int tempptr = tokenstart + 2; // skip "f "
			int forwardslashcount=0;
			bool adjacentslash = false;

			// this works out how many elements are specified for a face, e.g.
			// f 1 2 3				-> 0 forward slashes = just position
			// f 1/1 2/2 3/3		-> 3 slashes = position and texcoords
			// f 1/1/1 2/2/2 3/3/3	-> 6 slashes = position, texcoords, normals
			// f 1//1 2//2 3//3		-> 6 slashes with adjacent = position, normals
			while(fbuffer[tempptr] != '\n')
			{
				if(fbuffer[tempptr] == '/')
				{
					forwardslashcount++;
					if(fbuffer[tempptr-1] == '/') adjacentslash=true;
				}
				tempptr++;
			}

			success=true;

			// Get 3 sets of indices per face
			for(int i=0; i<3; i++)
			{
				// get vertex index
				success = success && getnexttoken(tokenstart, tokenlength);
				pindices.push_back(atoi(&fbuffer[tokenstart]));

				if(forwardslashcount>=3&& adjacentslash==false) // get texcoord index if required 
				{
					success = success && getnexttoken(tokenstart, tokenlength);
					tindices.push_back(atoi(&fbuffer[tokenstart]));
				}

				if(forwardslashcount==6 || adjacentslash==true) // get normal index if required 
				{
					success = success && getnexttoken(tokenstart, tokenlength);
					nindices.push_back(atoi(&fbuffer[tokenstart]));
				}
			}

			if(!success) {char s[100] = "ERROR: Badly formatted face, line : "; _itoa(line, &s[strlen(s)], 10); strcat(s, " : "); strcat(s, filename.c_str());  DXTRACE_MSG(s); }
		}
	} while(getnextline() == true);
}


// get next token. if \n is next token do not proceed, use getnextline() to resume
bool ObjFileModel::getnexttoken(int& tokenstart, int& tokenlength)
{
	tokenstart = tokenptr; 
	tokenlength=1; 
	int tokenend;

	while(fbuffer[tokenptr] == ' ' || fbuffer[tokenptr] == '\t' || fbuffer[tokenptr] == '/') tokenptr++; //skip whitespace and '/'

	if(fbuffer[tokenptr] == '\n') { return false; } // keeps tokenptr pointing to \n as a token to indicate end of line
													// doesn't point to next token, dealt with in getnextline()
	tokenend=tokenptr+1;

	while(fbuffer[tokenend] != ' ' && fbuffer[tokenend] != '\t' && fbuffer[tokenend] != '\n' && fbuffer[tokenend] != '/') tokenend++; // find length of token by finding next whitespace or '\n' or '/'

	tokenlength = tokenend - tokenptr;
	tokenstart = tokenptr;
	tokenptr+=tokenlength; //ready for next token

	return true;
}


// gets next line of buffer by skipping to next element after end of current line, returns false when end of buffer exceeded
bool ObjFileModel::getnextline()
{
	// relies on getnexttoken()leaving tokenptr pointing to \n if encountered

	while(fbuffer[tokenptr] != '\n' && tokenptr < actualsize) tokenptr++; // skip to end of line

	tokenptr++; // point to start of next line

	if (tokenptr >= actualsize) return false;
	else return true;
}


// create Vertex buffer from parsed file data
bool ObjFileModel::createVB()
{
	// create vertex array to pass to vertex buffer from parsed data
	numverts = pindices.size();

	vertices = new MODEL_POS_TEX_NORM_VERTEX[numverts]; // create big enough vertex array

	for(unsigned int i = 0; i< numverts; i++)
	{
		int vindex = pindices[i]-1; // use -1 for indices as .obj files indices begin at 1

		// set position data
		vertices[i].Pos.x = position_list[vindex].x;
		vertices[i].Pos.y = position_list[vindex].y;
		vertices[i].Pos.z = position_list[vindex].z;

		if(tindices.size() > 0)
		{ 
			// if there are any, set texture coord data
			int tindex = tindices[i]-1;
			vertices[i].TexCoord.x = texcoord_list[tindex].x;
			vertices[i].TexCoord.y = texcoord_list[tindex].y;
		}

		if(nindices.size() > 0)
		{
			// if there are any, set normal data
			int nindex = nindices[i]-1;
			vertices[i].Normal.x = normal_list[nindex].x;
			vertices[i].Normal.y = normal_list[nindex].y;
			vertices[i].Normal.z = normal_list[nindex].z;
		}
	}

	// Set up and create vertex buffer
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;										// Used by CPU and GPU
	bufferDesc.ByteWidth = sizeof(vertices[0])*numverts;						// Total size of buffer
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;							// Use as a vertex buffer
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;							// Allow CPU access
	HRESULT hr = pD3DDevice->CreateBuffer(&bufferDesc, NULL, &pVertexBuffer);	// Create the buffer

	if(FAILED(hr))
    {
        return false;
    }

	// Copy the vertices into the buffer
	D3D11_MAPPED_SUBRESOURCE ms;
	pImmediateContext->Map(pVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);	// Lock the buffer to allow writing
	memcpy(ms.pData, vertices, sizeof(vertices[0])*numverts);							// Copy the data
	pImmediateContext->Unmap(pVertexBuffer, NULL);										// Unlock the buffer

	return true;
}


ObjFileModel::~ObjFileModel()
{
	// clean up memory used by object
	if(pVertexBuffer) pVertexBuffer->Release();

	delete [] vertices;

	position_list.clear();
	normal_list.clear();
	texcoord_list.clear();
}

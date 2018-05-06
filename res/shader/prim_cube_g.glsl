#version 330 core

layout (lines) in;
layout (triangle_strip, max_vertices = 14) out;

void main() {    
    gl_Position = gl_in[0].gl_Position + vec4(-0.1, 0.0, 0.0, 0.0); 
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4( 0.1, 0.0, 0.0, 0.0);
    EmitVertex();
    
    EndPrimitive();





    fColor = gs_in[0].color; 
    gl_Position = position + vec4(-0.2, -0.2, 0.0, 0.0);    // 1:bottom-left   
    EmitVertex();   
    gl_Position = position + vec4( 0.2, -0.2, 0.0, 0.0);    // 2:bottom-right
    EmitVertex();
    gl_Position = position + vec4(-0.2,  0.2, 0.0, 0.0);    // 3:top-left
    EmitVertex();
    gl_Position = position + vec4( 0.2,  0.2, 0.0, 0.0);    // 4:top-right
    EmitVertex();
    gl_Position = position + vec4( 0.0,  0.4, 0.0, 0.0);    // 5:top
    fColor = vec3(1.0, 1.0, 1.0);
    EmitVertex();
    EndPrimitive();  



}  



////////////////////////////////////////



[maxvertexcount(24)]
void GSBoxQuads(point VertexOut gin[1], 
        uint primID : SV_PrimitiveID, 
        inout TriangleStream<GeoOut> triStream)
{	
	// calculate halfWidth and height to create cube vertices			==> ONLY USING SIZE X!
	float halfWidth		= 0.5f*gin[0].SizeW.x;
	float halfHeight	= 0.5f*gin[0].SizeW.x;
	float halfDepth		= 0.5f*gin[0].SizeW.x;
	
	// create the 24 vertices of the cube
	float4 boxv[24];
	boxv[0] = float4(gin[0].CenterW.x	-halfWidth, gin[0].CenterW.y	+halfHeight, gin[0].CenterW.z	-halfDepth, 1.0f);
	boxv[1] = float4(gin[0].CenterW.x	+halfWidth, gin[0].CenterW.y	+halfHeight, gin[0].CenterW.z	-halfDepth, 1.0f);
	boxv[2] = float4(gin[0].CenterW.x	-halfWidth, gin[0].CenterW.y	-halfHeight, gin[0].CenterW.z	-halfDepth, 1.0f);
	boxv[3] = float4(gin[0].CenterW.x	+halfWidth, gin[0].CenterW.y	-halfHeight, gin[0].CenterW.z	-halfDepth, 1.0f);
	boxv[4] = float4(gin[0].CenterW.x	+halfWidth, gin[0].CenterW.y	+halfHeight, gin[0].CenterW.z	-halfDepth, 1.0f);
	boxv[5] = float4(gin[0].CenterW.x	+halfWidth, gin[0].CenterW.y	+halfHeight, gin[0].CenterW.z	+halfDepth, 1.0f);
	boxv[6] = float4(gin[0].CenterW.x	+halfWidth, gin[0].CenterW.y	-halfHeight, gin[0].CenterW.z	-halfDepth, 1.0f);
	boxv[7] = float4(gin[0].CenterW.x	+halfWidth, gin[0].CenterW.y	-halfHeight, gin[0].CenterW.z	+halfDepth, 1.0f);
	boxv[8] = float4(gin[0].CenterW.x	+halfWidth, gin[0].CenterW.y	+halfHeight, gin[0].CenterW.z	+halfDepth, 1.0f);
	boxv[9] = float4(gin[0].CenterW.x	-halfWidth, gin[0].CenterW.y	+halfHeight, gin[0].CenterW.z	+halfDepth, 1.0f);
	boxv[10] = float4(gin[0].CenterW.x	+halfWidth, gin[0].CenterW.y	-halfHeight, gin[0].CenterW.z	+halfDepth, 1.0f);
	boxv[11] = float4(gin[0].CenterW.x	-halfWidth, gin[0].CenterW.y	-halfHeight, gin[0].CenterW.z	+halfDepth, 1.0f);
	boxv[12] = float4(gin[0].CenterW.x	-halfWidth, gin[0].CenterW.y	+halfHeight, gin[0].CenterW.z	+halfDepth, 1.0f);
	boxv[13] = float4(gin[0].CenterW.x	-halfWidth, gin[0].CenterW.y	+halfHeight, gin[0].CenterW.z	-halfDepth, 1.0f);
	boxv[14] = float4(gin[0].CenterW.x	-halfWidth, gin[0].CenterW.y	-halfHeight, gin[0].CenterW.z	+halfDepth, 1.0f);
	boxv[15] = float4(gin[0].CenterW.x	-halfWidth, gin[0].CenterW.y	-halfHeight, gin[0].CenterW.z	-halfDepth, 1.0f);
	boxv[16] = float4(gin[0].CenterW.x	-halfWidth, gin[0].CenterW.y	+halfHeight, gin[0].CenterW.z	+halfDepth, 1.0f);
	boxv[17] = float4(gin[0].CenterW.x	+halfWidth, gin[0].CenterW.y	+halfHeight, gin[0].CenterW.z	+halfDepth, 1.0f);
	boxv[18] = float4(gin[0].CenterW.x	-halfWidth, gin[0].CenterW.y	+halfHeight, gin[0].CenterW.z	-halfDepth, 1.0f);
	boxv[19] = float4(gin[0].CenterW.x	+halfWidth, gin[0].CenterW.y	+halfHeight, gin[0].CenterW.z	-halfDepth, 1.0f);
	boxv[20] = float4(gin[0].CenterW.x	-halfWidth, gin[0].CenterW.y	-halfHeight, gin[0].CenterW.z	-halfDepth, 1.0f);
	boxv[21] = float4(gin[0].CenterW.x	+halfWidth, gin[0].CenterW.y	-halfHeight, gin[0].CenterW.z	-halfDepth, 1.0f);
	boxv[22] = float4(gin[0].CenterW.x	-halfWidth, gin[0].CenterW.y	-halfHeight, gin[0].CenterW.z	+halfDepth, 1.0f);
	boxv[23] = float4(gin[0].CenterW.x	+halfWidth, gin[0].CenterW.y	-halfHeight, gin[0].CenterW.z	+halfDepth, 1.0f);

	// indices 6 quads * 4 vertices
	int index[24] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};
	
	// after each quad, restart the strip
	GeoOut gout;
	[unroll]
	for(int q1=1;q1<25;++q1)
	{
		gout.PosH     = mul(boxv[index[q1-1]], gViewProj);
		gout.PosW     = boxv[index[q1-1]].xyz;
		gout.NormalW  = gNormals[index[q1-1]];
		gout.Tex      = gTexC[index[q1-1]];
		gout.PrimID   = primID;
		
		triStream.Append(gout);

		if(q1 % 4 == 0) triStream.RestartStrip();
	}
}


[maxvertexcount(14)]
void GSBoxStrip(point VertexOut gin[1], 
        uint primID : SV_PrimitiveID, 
        inout TriangleStream<GeoOut> triStream)
{	
	// calculate halfWidth and height to create cube vertices			==> ONLY USING SIZE X!
	float halfWidth		= 0.5f*gin[0].SizeW.x;
	float halfHeight	= 0.5f*gin[0].SizeW.x;
	float halfDepth		= 0.5f*gin[0].SizeW.x;
	
	// create the 8 vertices of the cube
	float4 boxv[14];
	boxv[0] = float4(gin[0].CenterW.x	-halfWidth, gin[0].CenterW.y	+halfHeight, gin[0].CenterW.z	-halfDepth, 1.0f);
	boxv[1] = float4(gin[0].CenterW.x	+halfWidth, gin[0].CenterW.y	+halfHeight, gin[0].CenterW.z	-halfDepth, 1.0f);
	boxv[2] = float4(gin[0].CenterW.x	-halfWidth, gin[0].CenterW.y	-halfHeight, gin[0].CenterW.z	-halfDepth, 1.0f);
	boxv[3] = float4(gin[0].CenterW.x	+halfWidth, gin[0].CenterW.y	-halfHeight, gin[0].CenterW.z	-halfDepth, 1.0f);
	boxv[4] = float4(gin[0].CenterW.x	+halfWidth, gin[0].CenterW.y	-halfHeight, gin[0].CenterW.z	+halfDepth, 1.0f);
	boxv[5] = float4(gin[0].CenterW.x	+halfWidth, gin[0].CenterW.y	+halfHeight, gin[0].CenterW.z	-halfDepth, 1.0f);
	boxv[6] = float4(gin[0].CenterW.x	+halfWidth, gin[0].CenterW.y	+halfHeight, gin[0].CenterW.z	+halfDepth, 1.0f);
	boxv[7] = float4(gin[0].CenterW.x	-halfWidth, gin[0].CenterW.y	+halfHeight, gin[0].CenterW.z	-halfDepth, 1.0f);
	boxv[8] = float4(gin[0].CenterW.x	-halfWidth, gin[0].CenterW.y	+halfHeight, gin[0].CenterW.z	+halfDepth, 1.0f);
	boxv[9] = float4(gin[0].CenterW.x	-halfWidth, gin[0].CenterW.y	-halfHeight, gin[0].CenterW.z	-halfDepth, 1.0f);
	boxv[10] = float4(gin[0].CenterW.x	-halfWidth, gin[0].CenterW.y	-halfHeight, gin[0].CenterW.z	+halfDepth, 1.0f);
	boxv[11] = float4(gin[0].CenterW.x	+halfWidth, gin[0].CenterW.y	-halfHeight, gin[0].CenterW.z	+halfDepth, 1.0f);
	boxv[12] = float4(gin[0].CenterW.x	-halfWidth, gin[0].CenterW.y	+halfHeight, gin[0].CenterW.z	+halfDepth, 1.0f);
	boxv[13] = float4(gin[0].CenterW.x	+halfWidth, gin[0].CenterW.y	+halfHeight, gin[0].CenterW.z	+halfDepth, 1.0f);

	int index[14] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};

	GeoOut gout;
	[unroll]
	for(int i=0;i<14;++i)
	{
		gout.PosH     = mul(boxv[index[i]], gViewProj);
		gout.PosW     = boxv[index[i]].xyz;
		gout.NormalW  = gNormalsStrip[index[i]];
		gout.Tex      = gTexCStrip[index[i]];
		gout.PrimID   = primID;
		
		triStream.Append(gout);
	}
}
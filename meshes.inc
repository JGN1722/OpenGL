struct MESH
	vao dd ?
	vbo dd ?
	ebo dd ?
	uv  dd ?
	n_vertices  dd ?
	n_triangles dd ?
	normals dd ?
ends

	; Outputs an icosphere mesh
	; Arguments:
	; - x, y, z
	; - subdivision count
	; - mesh address
GetIcosphereMesh: ; TODO: add scale argument
	push	ebp
	mov	ebp, esp
	
	mov	eax, 12 * 3 * 4
	shl	eax, 1 ; 1 subdivision
	
	invoke	HeapAlloc, [hHeap], HEAP_ZERO_MEMORY, eax
	pushd	eax
	
	mov	edi, eax
	mov	esi, vertices
	mov	ecx, 12 * 3
	
	rep movsd
	
	; Generate the new vertices
	; ...
	
	mov	ecx, 0
	mov	edi, DWORD [ebp - 4]
	
      @@:
	
	fld	DWORD [edi]
	fadd	DWORD [ebp + 8]
	fstp	DWORD [edi]
	
	fld	DWORD [edi + 4]
	fadd	DWORD [ebp + 12]
	fstp	DWORD [edi + 4]
	
	fld	DWORD [edi + 8]
	fadd	DWORD [ebp + 16]
	fstp	DWORD [edi + 8]
	
	add	edi, 3 * 4
	inc	ecx
	
	cmp	ecx, 12
	jne	@b
	
	mov	ebx, DWORD [ebp + 24]
	
	invoke	glGenVertexArrays, 1, ebx
	add	ebx, 4
	invoke	glGenBuffers, 1, ebx
	add	ebx, 4
	invoke	glGenBuffers, 1, ebx
	add	ebx, 4
	invoke	glGenBuffers, 1, ebx
	
	mov	ebx, DWORD [ebp + 24]
	
	invoke	glBindVertexArray, [ebx]
	
	invoke	glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, [ebx + 8]
	invoke	glBufferData, GL_ELEMENT_ARRAY_BUFFER, 20 * 3 * 4, indices, GL_STATIC_DRAW
	
	invoke	glBindBuffer, GL_ARRAY_BUFFER, [ebx + 4]
	invoke	glBufferData, GL_ARRAY_BUFFER, 12 * 3 * 4, DWORD [ebp - 4], GL_STATIC_DRAW
	
	invoke	glBindBuffer, GL_ARRAY_BUFFER, [ebx + 12]
	invoke	glBufferData, GL_ARRAY_BUFFER, 12 * 2 * 4, uv_data, GL_STATIC_DRAW
	
	mov	DWORD [ebx + 16], 12
	mov	DWORD [ebx + 20], 20
	
	push	ebx
	stdcall	ComputeNormals, ebx
	pop	ebx
	
	invoke	glBindBuffer, GL_ARRAY_BUFFER, [ebx + 4]
	invoke	glEnableVertexAttribArray, 0
	invoke	glVertexAttribPointer, 0, 3, GL_FLOAT, GL_FALSE, 3 * 4, NULL
	
	invoke	glBindBuffer, GL_ARRAY_BUFFER, [ebx + 12]
	invoke	glEnableVertexAttribArray, 1
	invoke	glVertexAttribPointer, 1, 2, GL_FLOAT, GL_FALSE, 2 * 4, NULL
	
	invoke	glBindBuffer, GL_ARRAY_BUFFER, 0
	invoke	glBindVertexArray, 0
	
	invoke	HeapFree, [hHeap], 0, DWORD [ebp - 4]
	
	mov	esp, ebp
	pop	ebp
	ret	20

	; Generate a cube mesh
	; Arguments:
	; - x, y, z
	; - size
	; - mesh address
GetCubeMesh:
	push	ebp
	mov	ebp, esp
	
	invoke	HeapAlloc, [hHeap], HEAP_ZERO_MEMORY, 8 * 3 * 4
	pushd	eax
	
	mov	edi, eax
	mov	esi, cube_vertices
	mov	ecx, 8 * 3
	
	rep movsd
	
	mov	ecx, 0
	mov	edi, DWORD [ebp - 4]
	
      @@:
	
	push	ecx edi
	
	; Apply scale
	stdcall	vec3Scale,\
		DWORD [edi], DWORD [edi + 4], DWORD [edi + 8],\
		DWORD [ebp + 20], edi
	
	pop	edi ecx
	
	; Apply translation
	stdcall	AddMulVec3,\
		[edi], [edi + 4], [edi + 8],\
		[ebp + 8], [ebp + 12], [ebp + 16],\
		1.0, edi
	
	add	edi, 3 * 4
	inc	ecx
	
	cmp	ecx, 8
	jne	@b
	
	mov	ebx, DWORD [ebp + 24]
	
	invoke	glGenVertexArrays, 1, ebx
	add	ebx, 4
	invoke	glGenBuffers, 1, ebx
	add	ebx, 4
	invoke	glGenBuffers, 1, ebx
	add	ebx, 4
	invoke	glGenBuffers, 1, ebx
	
	mov	ebx, DWORD [ebp + 24]
	
	invoke	glBindVertexArray, [ebx]
	
	invoke	glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, [ebx + 8]
	invoke	glBufferData, GL_ELEMENT_ARRAY_BUFFER, 12 * 3 * 4, cube_indices, GL_STATIC_DRAW
	
	invoke	glBindBuffer, GL_ARRAY_BUFFER, [ebx + 4]
	invoke	glBufferData, GL_ARRAY_BUFFER, 8 * 3 * 4, DWORD [ebp - 4], GL_STATIC_DRAW
	
	invoke	glBindBuffer, GL_ARRAY_BUFFER, [ebx + 12]
	invoke	glBufferData, GL_ARRAY_BUFFER, 8 * 2 * 4, cube_uv, GL_STATIC_DRAW
	
	mov	DWORD [ebx + 16], 8
	mov	DWORD [ebx + 20], 12
	
	push	ebx
	stdcall	ComputeNormals, ebx
	pop	ebx
	
	invoke	glBindBuffer, GL_ARRAY_BUFFER, [ebx + 4]
	invoke	glEnableVertexAttribArray, 0
	invoke	glVertexAttribPointer, 0, 3, GL_FLOAT, GL_FALSE, 3 * 4, NULL
	
	invoke	glBindBuffer, GL_ARRAY_BUFFER, [ebx + 12]
	invoke	glEnableVertexAttribArray, 1
	invoke	glVertexAttribPointer, 1, 2, GL_FLOAT, GL_FALSE, 2 * 4, NULL
	
	invoke	glBindBuffer, GL_ARRAY_BUFFER, 0
	invoke	glBindVertexArray, 0
	
	invoke	HeapFree, [hHeap], 0, DWORD [ebp - 4]
	mov	esp, ebp
	pop	ebp
	ret	20

	; Generates a cylinder mesh
	; Arguments:
	; - x, y, z
	; - height
	; - radius
	; - height subdivisions ( int )
	; - radial subdivisions ( int )
	; - mesh address
GetCylinderMesh:
	push	ebp
	mov	ebp, esp
	
	; Vertices count = Height subdiv * Radial subdiv + 2
	mov	eax, DWORD [ebp + 28]
	mul	DWORD [ebp + 32]
	add	eax, 2 ; top and bottom central points
	
	mov	ebx, DWORD [ebp + 36]
	mov	DWORD [ebx + 16], eax
	
	mov	ebx, 3 * 4
	mul	ebx
	
	invoke	HeapAlloc, [hHeap], HEAP_ZERO_MEMORY, eax
	push	eax
	
	; Triangle count = (Height subdiv - 1) * 2 * Radial subdiv + 2 * Radial subdiv
	;		 = 2 * Height subdiv * Radial subdiv
	mov	eax, DWORD [ebp + 28]
	
	mul	DWORD [ebp + 32]
	shl	eax, 1
	
	mov	ebx, DWORD [ebp + 36]
	mov	DWORD [ebx + 20], eax
	
	mov	ebx, 3 * 4
	mul	ebx
	
	invoke	HeapAlloc, [hHeap], HEAP_ZERO_MEMORY, eax
	push	eax
	
	mov	edi, DWORD [ebp - 4]
	mov	esi, DWORD [ebp - 8]
	
	mov	ecx, 0
	push	ecx
	
      .new_height_step:
	
	fild	DWORD [esp]
	fmul	DWORD [ebp + 20]
	
	fild	DWORD [ebp + 28]
	fld1
	fsubp	st1, st0
	
	fdivp	st1, st0
	
	sub	esp, 4
	
	fstp	DWORD [esp]
	pop	ebx ; ebx holds y for this step
	
	; Calculate the delta angle
	; TODO: can be calculated before the outer loop
	fldpi
	
	pushd	2.0
	fmul	DWORD [esp]
	
	fidiv	DWORD [ebp + 32]
	fstp	DWORD [esp]
	
      .generate_height_step:
	mov	ecx, 0
	push	ecx ; This might not be necessary
	
       .new_vertice:
	fild	DWORD [esp]
	fmul	DWORD [esp + 4]
	fsincos
	
	fld	DWORD [ebp + 24]
	fmul	st1, st0
	fmulp	st2, st0
	
	fstp	DWORD [edi]
	mov	DWORD [edi + 4], ebx
	fstp	DWORD [edi + 8]
	
	push	ecx edi esi ebx
	
	stdcall AddMulVec3,\
		[ebp +  8], [ebp + 12], [ebp + 16],\
		[edi     ], [edi +  4], [edi +  8], 1.0, edi
	
	pop	ebx esi edi ecx
	
	add	edi, 3 * 4
	
	; We also generate the triangles at the same time, to avoid having
	; to write two loops
	mov	eax, DWORD [ebp + 28]
	dec	eax
	cmp	eax, DWORD [ebp - 12]
	
	je	.triangles_end ; don't create triangles for the last row
	
	mov	eax, DWORD [ebp - 12]
	mul	DWORD [ebp + 32]
	
	add	eax, ecx
	push	eax
	
	; First triangle
	mov	DWORD [esi], eax
	
	inc	eax
	
	push	eax
	xor	edx, edx
	
	div	DWORD [ebp + 32]
	pop	eax
	
	cmp	edx, 0
	jne	@f
	
	sub	eax, DWORD [ebp + 32] ; Wrap around when we get to the last point
       @@:
	
	mov	DWORD [esi + 8], eax
	
	add	eax, DWORD [ebp + 32]
	mov	DWORD [esi + 4], eax
	
	pop	eax
	
	; Second triangle
	mov	DWORD [esi + 12], eax
	
	add	eax, DWORD [ebp + 32]
	mov	DWORD [esi + 16], eax
	
	inc	eax
	
	push	eax
	xor	edx, edx
	
	div	DWORD [ebp + 32]
	pop	eax
	
	cmp	edx, 0
	jne	@f
	
	sub	eax, DWORD [ebp + 32]
       @@:
	
	mov	DWORD [esi + 20], eax
	
	add	esi, 24
	
       .triangles_end:
	
	pop	ecx
	inc	ecx
	push	ecx
	
	cmp	ecx, DWORD [ebp + 32]
	jne	.new_vertice
	
	add	esp, 8 ; Clean up delta angle and counter
	
	pop	ecx
	inc	ecx
	push	ecx
	
	cmp	ecx, DWORD [ebp + 28]
	jne	.new_height_step
	
	add	esp, 4
	
	; Add the top and bottom central points in the vertices buffer
	mov	eax, [ebp + 8]
	mov	[edi], eax
	mov	eax, [ebp + 12]
	mov	[edi + 4], eax
	mov	eax, [ebp + 16]
	mov	[edi + 8], eax
	
	add	edi, 12
	
	mov	eax, [ebp + 8]
	mov	[edi], eax
	mov	eax, [ebp + 16]
	mov	[edi + 8], eax
	
	fld	DWORD [ebp + 12]
	fadd	DWORD [ebp + 20]
	fstp	DWORD [edi + 4]
	
	; Add the top and bottom triangles
	mov	ecx, 0
	push	ecx
	
	mov	eax, DWORD [ebp + 28]
	dec	eax
	mul	DWORD [ebp + 32]
	mov	edx, eax
	
	mov	ebx, DWORD [ebp + 36]
	mov	eax, DWORD [ebx + 16]
	sub	eax, 2
	
	mov	DWORD [esi], 0
	mov	DWORD [esi + 4], 1
	mov	DWORD [esi + 8], eax
	
	
      .new_triangle:
	lea	ebx, [ecx + 1]
	
	cmp	ebx, DWORD [ebp + 32]
	jne	.no_wraparound
	
	mov	ebx, 0
       .no_wraparound:
	
	mov	DWORD [esi], ecx
	mov	DWORD [esi + 4], ebx
	mov	DWORD [esi + 8], eax
	
	add	esi, 12
	
	inc	eax
	add	ecx, edx
	add	ebx, edx
	
	mov	DWORD [esi], ecx
	mov	DWORD [esi + 4], ebx
	mov	DWORD [esi + 8], eax
	
	dec	eax
	
	add	esi, 12
	
	pop	ecx
	inc	ecx
	push	ecx
	
	cmp	ecx, DWORD [ebp + 32]
	jne	.new_triangle
	
	add	esp, 4
	
	; Create the UV buffer
	mov	ebx, DWORD [ebp + 36]
	mov	eax, [ebx + 16]
	mov	ebx, eax
	
	shl	eax, 3 ; *= 2 * 4
	
	invoke	HeapAlloc, [hHeap], HEAP_ZERO_MEMORY, eax
	push	eax
	
	mov	ecx, 0
	mov	edi, eax
	
	mov	dl, 0
	
      @@:
	mov	eax, 0.0
	
	cmp	dl, 0
	je	.nochange
	
	mov	eax, 1.0
       .nochange:
	mov	DWORD [edi], eax
	mov	DWORD [edi + 4], eax
	
	add	edi, 8
	not	dl
	
	inc	ecx
	
	cmp	ecx, ebx
	jne	@b
	
	; Create the mesh
	mov	ebx, DWORD [ebp + 36]
	
	invoke	glGenVertexArrays, 1, ebx
	add	ebx, 4
	invoke	glGenBuffers, 1, ebx
	add	ebx, 4
	invoke	glGenBuffers, 1, ebx
	add	ebx, 4
	invoke	glGenBuffers, 1, ebx
	
	mov	ebx, DWORD [ebp + 36]
	
	invoke	glBindVertexArray, [ebx]
	
	invoke	glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, [ebx + 8]
	
	mov	eax, DWORD [ebx + 20]
	mov	ecx, 3 * 4
	mul	ecx
	
	invoke	glBufferData, GL_ELEMENT_ARRAY_BUFFER, eax, DWORD [ebp - 8], GL_STATIC_DRAW
	
	invoke	glBindBuffer, GL_ARRAY_BUFFER, [ebx + 4]
	
	mov	eax, DWORD [ebx + 16]
	mov	ecx, 3 * 4
	mul	ecx
	
	invoke	glBufferData, GL_ARRAY_BUFFER, eax, DWORD [ebp - 4], GL_STATIC_DRAW
	
	invoke	glBindBuffer, GL_ARRAY_BUFFER, [ebx + 12]
	
	mov	eax, DWORD [ebx + 16]
	mov	ecx, 2 * 4
	mul	ecx
	
	invoke	glBufferData, GL_ARRAY_BUFFER, eax, DWORD [ebp - 12], GL_STATIC_DRAW
	
	push	ebx
	stdcall	ComputeNormals, ebx
	pop	ebx
	
	invoke	glBindBuffer, GL_ARRAY_BUFFER, [ebx + 4]
	invoke	glEnableVertexAttribArray, 0
	invoke	glVertexAttribPointer, 0, 3, GL_FLOAT, GL_FALSE, 3 * 4, NULL
	
	invoke	glBindBuffer, GL_ARRAY_BUFFER, [ebx + 12]
	invoke	glEnableVertexAttribArray, 1
	invoke	glVertexAttribPointer, 1, 2, GL_FLOAT, GL_FALSE, 2 * 4, NULL
	
	invoke	glBindBuffer, GL_ARRAY_BUFFER, 0
	invoke	glBindVertexArray, 0
	
	invoke	HeapFree, [hHeap], HEAP_ZERO_MEMORY, DWORD [ebp - 4]
	invoke	HeapFree, [hHeap], HEAP_ZERO_MEMORY, DWORD [ebp - 8]
	invoke	HeapFree, [hHeap], HEAP_ZERO_MEMORY, DWORD [ebp - 12]
	
	mov	esp, ebp
	pop	ebp
	ret	32

	; Generates a UV sphere mesh
	; Arguments:
	; - x, y, z
	; - radius
	; - horizontal subdivisions ( int )
	; - vertical subdivisions ( int )
	; - mesh address
GetUVSphereMesh:
	push	ebp
	mov	ebp, esp
	
	; Vertices count = Horizontal subdiv * Vertical subdiv + 2
	mov	eax, DWORD [ebp + 24]
	mul	DWORD [ebp + 28]
	add	eax, 2 ; top and bottom central points
	
	mov	ebx, DWORD [ebp + 32]
	mov	DWORD [ebx + 16], eax
	
	mov	ebx, 3 * 4
	mul	ebx
	
	invoke	HeapAlloc, [hHeap], HEAP_ZERO_MEMORY, eax
	push	eax
	
	; Triangle count = (Horizontal subdiv - 1) * 2 * vertical subdiv + 2 * vertical subdiv
	;		 = 2 * Horizontal subdiv * vertical subdiv
	mov	eax, DWORD [ebp + 24]
	
	mul	DWORD [ebp + 28]
	shl	eax, 1
	
	mov	ebx, DWORD [ebp + 32]
	mov	DWORD [ebx + 20], eax
	
	mov	ebx, 3 * 4
	mul	ebx
	
	invoke	HeapAlloc, [hHeap], HEAP_ZERO_MEMORY, eax
	push	eax
	
	mov	edi, DWORD [ebp - 4]
	mov	esi, DWORD [ebp - 8]
	
	mov	ecx, 0
	push	ecx
	
      .new_height_step:
	
	fild	DWORD [esp]
	fld1
	faddp	st1, st0
	fmul	DWORD [ebp + 20]
	
	fild	DWORD [ebp + 24]
	fld1
	faddp	st1, st0
	
	fdivp	st1, st0
	
	pushd	2.0
	fmul	DWORD [esp]
	
	fld	DWORD [ebp + 20]
	fsubp	st1, st0
	
	fstp	DWORD [esp]
	pop	ebx ; ebx holds y for this step
	
	; Calculate the delta angle
	; TODO: can be calculated before the outer loop
	fldpi
	
	pushd	2.0
	fmul	DWORD [esp]
	
	fidiv	DWORD [ebp + 28]
	fstp	DWORD [esp]
	
      .generate_height_step:
	mov	ecx, 0
	push	ecx ; This might not be necessary
	
       .new_vertice:
	fild	DWORD [esp]
	fmul	DWORD [esp + 4]
	fsincos
	
	fld	DWORD [ebp + 20]
	fmul	st1, st0
	fmulp	st2, st0
	
	fstp	DWORD [edi]
	fstp	DWORD [edi + 8]
	
	push	ecx edi esi ebx
	
	push	ebx
	
	; TODO: we could normalize y, then square
	fld	DWORD [esp]
	fld	DWORD [esp]
	fmulp	st1, st0
	
	fdiv	DWORD [ebp + 20]
	fdiv	DWORD [ebp + 20]
	
	fld1
	fsubrp	st1, st0
	
	fsqrt
	fstp	DWORD [esp]
	pop	eax
	
	stdcall	vec3Scale, [edi], [edi + 4], [edi + 8], eax, edi
	
	pop	ebx esi edi ecx
	
	mov	DWORD [edi + 4], ebx
	
	; Apply translation
	push	ecx edi esi ebx
	
	stdcall AddMulVec3,\
		[ebp +  8], [ebp + 12], [ebp + 16],\
		[edi     ], [edi +  4], [edi +  8], 1.0, edi
	
	pop	ebx esi edi ecx
	
	add	edi, 3 * 4
	
	; We also generate the triangles at the same time, to avoid having
	; to write two loops
	mov	eax, DWORD [ebp + 24]
	dec	eax
	cmp	eax, DWORD [ebp - 12]
	
	je	.triangles_end ; don't create triangles for the last row
	
	mov	eax, DWORD [ebp - 12]
	mul	DWORD [ebp + 28]
	
	add	eax, ecx
	push	eax
	
	; First triangle
	mov	DWORD [esi], eax
	
	inc	eax
	
	push	eax
	xor	edx, edx
	
	div	DWORD [ebp + 28]
	pop	eax
	
	cmp	edx, 0
	jne	@f
	
	sub	eax, DWORD [ebp + 28] ; Wrap around when we get to the last point
       @@:
	
	mov	DWORD [esi + 8], eax
	
	add	eax, DWORD [ebp + 28]
	mov	DWORD [esi + 4], eax
	
	pop	eax
	
	; Second triangle
	mov	DWORD [esi + 12], eax
	
	add	eax, DWORD [ebp + 28]
	mov	DWORD [esi + 16], eax
	
	inc	eax
	
	push	eax
	xor	edx, edx
	
	div	DWORD [ebp + 28]
	pop	eax
	
	cmp	edx, 0
	jne	@f
	
	sub	eax, DWORD [ebp + 28]
       @@:
	
	mov	DWORD [esi + 20], eax
	
	add	esi, 24
	
       .triangles_end:
	
	pop	ecx
	inc	ecx
	push	ecx
	
	cmp	ecx, DWORD [ebp + 28]
	jne	.new_vertice
	
	add	esp, 8 ; Clean up delta angle and counter
	
	pop	ecx
	inc	ecx
	push	ecx
	
	cmp	ecx, DWORD [ebp + 24]
	jne	.new_height_step
	
	add	esp, 4
	
	; Add the top and bottom central points in the vertices buffer
	mov	eax, [ebp + 8]
	mov	[edi], eax
	mov	eax, [ebp + 16]
	mov	[edi + 8], eax
	
	fld	DWORD [ebp + 12]
	fsub	DWORD [ebp + 20]
	fstp	DWORD [edi + 4]
	
	add	edi, 12
	
	mov	eax, [ebp + 8]
	mov	[edi], eax
	mov	eax, [ebp + 16]
	mov	[edi + 8], eax
	
	fld	DWORD [ebp + 12]
	fadd	DWORD [ebp + 20]
	fstp	DWORD [edi + 4]
	
	; Add the top and bottom triangles
	mov	ecx, 0
	push	ecx
	
	mov	eax, DWORD [ebp + 24]
	dec	eax
	mul	DWORD [ebp + 28]
	mov	edx, eax
	
	mov	ebx, DWORD [ebp + 32]
	mov	eax, DWORD [ebx + 16]
	sub	eax, 2
	
	mov	DWORD [esi], 0
	mov	DWORD [esi + 4], 1
	mov	DWORD [esi + 8], eax
	
	
      .new_triangle:
	lea	ebx, [ecx + 1]
	
	cmp	ebx, DWORD [ebp + 28]
	jne	.no_wraparound
	
	mov	ebx, 0
       .no_wraparound:
	
	mov	DWORD [esi], ecx
	mov	DWORD [esi + 4], ebx
	mov	DWORD [esi + 8], eax
	
	add	esi, 12
	
	inc	eax
	add	ecx, edx
	add	ebx, edx
	
	mov	DWORD [esi], ecx
	mov	DWORD [esi + 4], ebx
	mov	DWORD [esi + 8], eax
	
	dec	eax
	
	add	esi, 12
	
	pop	ecx
	inc	ecx
	push	ecx
	
	cmp	ecx, DWORD [ebp + 28]
	jne	.new_triangle
	
	add	esp, 4
	
	; Create the UV buffer
	mov	ebx, DWORD [ebp + 32]
	mov	eax, [ebx + 16]
	mov	ebx, eax
	
	shl	eax, 3 ; *= 2 * 4
	
	invoke	HeapAlloc, [hHeap], HEAP_ZERO_MEMORY, eax
	push	eax
	
	mov	ecx, 0
	mov	edi, eax
	
	mov	dl, 0
	
      @@:
	mov	eax, 0.0
	
	cmp	dl, 0
	je	.nochange
	
	mov	eax, 0.5
       .nochange:
	mov	DWORD [edi], eax
	mov	DWORD [edi + 4], eax
	
	add	edi, 8
	not	dl
	
	inc	ecx
	
	cmp	ecx, ebx
	jne	@b
	
	; Create the mesh
	mov	ebx, DWORD [ebp + 32]
	
	invoke	glGenVertexArrays, 1, ebx
	add	ebx, 4
	invoke	glGenBuffers, 1, ebx
	add	ebx, 4
	invoke	glGenBuffers, 1, ebx
	add	ebx, 4
	invoke	glGenBuffers, 1, ebx
	
	mov	ebx, DWORD [ebp + 32]
	
	invoke	glBindVertexArray, [ebx]
	
	invoke	glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, [ebx + 8]
	
	mov	eax, DWORD [ebx + 20]
	mov	ecx, 3 * 4
	mul	ecx
	
	invoke	glBufferData, GL_ELEMENT_ARRAY_BUFFER, eax, DWORD [ebp - 8], GL_STATIC_DRAW
	
	invoke	glBindBuffer, GL_ARRAY_BUFFER, [ebx + 4]
	
	mov	eax, DWORD [ebx + 16]
	mov	ecx, 3 * 4
	mul	ecx
	
	invoke	glBufferData, GL_ARRAY_BUFFER, eax, DWORD [ebp - 4], GL_STATIC_DRAW
	
	invoke	glBindBuffer, GL_ARRAY_BUFFER, [ebx + 12]
	
	mov	eax, DWORD [ebx + 16]
	shl	eax, 3 ; *= 2 * 4
	
	invoke	glBufferData, GL_ARRAY_BUFFER, eax, DWORD [ebp - 12], GL_STATIC_DRAW
	
	push	ebx
	stdcall	ComputeNormals, ebx
	pop	ebx
	
	invoke	glBindBuffer, GL_ARRAY_BUFFER, [ebx + 4]
	invoke	glEnableVertexAttribArray, 0
	invoke	glVertexAttribPointer, 0, 3, GL_FLOAT, GL_FALSE, 3 * 4, NULL
	
	invoke	glBindBuffer, GL_ARRAY_BUFFER, [ebx + 12]
	invoke	glEnableVertexAttribArray, 1
	invoke	glVertexAttribPointer, 1, 2, GL_FLOAT, GL_FALSE, 2 * 4, NULL
	
	invoke	glBindBuffer, GL_ARRAY_BUFFER, 0
	invoke	glBindVertexArray, 0
	
	invoke	HeapFree, [hHeap], HEAP_ZERO_MEMORY, DWORD [ebp - 4]
	invoke	HeapFree, [hHeap], HEAP_ZERO_MEMORY, DWORD [ebp - 8]
	invoke	HeapFree, [hHeap], HEAP_ZERO_MEMORY, DWORD [ebp - 12]
	
	mov	esp, ebp
	pop	ebp
	ret	28

	; Fills the vertex normals buffer of a mesh
	; Arguments:
	; - mesh address
ComputeNormals:
	push	ebp
	mov	ebp, esp
	
	mov	eax, DWORD [ebp + 8]
	mov	eax, DWORD [eax + 16]
	
	mov	ebx, 3 * 4
	mul	ebx
	
	invoke	HeapAlloc, [hHeap], HEAP_ZERO_MEMORY, eax
	push	eax
	
	; for every vertex (done)
	; - find triangles (done)
	; - compute triangle normal (todo)
	; - add triangle normal to final normal (todo)
	
	push	0
	
	mov	ebx, DWORD [ebp + 8]
	mov	ebx, DWORD [ebx + 8]
	invoke	glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, ebx
	
	mov	eax, DWORD [ebp + 8]
	mov	eax, DWORD [eax + 20]
	
	mov	ebx, 4 * 3
	mul	ebx
	
	invoke	glMapBufferRange, GL_ELEMENT_ARRAY_BUFFER, 0, eax, GL_MAP_READ_BIT
	push	eax
	
      @@:
	mov	ebx, DWORD [ebp + 8]
	stdcall	GetTriangleIndex, [ebp - 12], [ebx + 20], [ebp - 8]
	
	; Create the total normal (done)
	sub	esp, 12
	mov	DWORD [ebp - 16], 0.0
	mov	DWORD [ebp - 20], 0.0
	mov	DWORD [ebp - 24], 0.0
	
	; For each ecx: (done)
	; - Get the triangle (todo)
	; - Compute the normal (todo)
	; - Add it to the total (todo)
	push	ecx
	
      .inner_loop:
	; ...
	
	dec	DWORD [ebp - 28]
	cmp	DWORD [ebp - 28], 0
	jne	.inner_loop
	
	add	esp, 4
	
	; Store the computed normal (done)
	mov	eax, DWORD [ebp - 8]
	mov	ebx, 3 * 4
	mul	ebx
	
	add	eax, DWORD [ebp - 4]
	
	pop	DWORD [eax]
	pop	DWORD [eax + 4]
	pop	DWORD [eax + 12]
	
	inc	DWORD [ebp - 8]
	
	mov	ebx, DWORD [ebp + 8]
	mov	ebx, DWORD [ebx + 16]
	cmp	DWORD [ebp - 8], ebx
	jb	@b
	
	
	invoke	glUnmapBuffer, GL_ELEMENT_ARRAY_BUFFER
	
	; Store the buffer in the mesh (todo)
	
	mov	esp, ebp
	pop	ebp
	ret	4

	; Gets the list of triangles that contains the specified vertex
	; Returns a buffer containing the list of the indexes of these
	; triangles in eax, and the count of them in ecx
	; Arguments:
	; - triangle buffer
	; - total triangle count
	; - vertex to look for
GetTriangleIndex:
	push	ebp
	mov	ebp, esp
	
	; There is at most 'triangle count' triangles to be returned
	mov	eax, DWORD [ebp + 12]
	shl	eax, 2 ; 4 bytes per triangle index
	
	invoke	HeapAlloc, [hHeap], HEAP_ZERO_MEMORY, eax
	push	eax
	
	push	0
	mov	ecx, 0
	mov	esi, DWORD [ebp + 8]
	mov	edi, eax
	
      @@:
	mov	eax, DWORD [ebp + 16]
	
	cmp	[esi], eax
	je	.found
	cmp	[esi + 4], eax
	je	.found
	cmp	[esi + 8], eax
	je	.found
	
	jmp	.not_found
	
      .found:
	mov	eax, [ebp - 8]
	mov	[edi], eax
	add	edi, 4
	
	inc	ecx
	
      .not_found:
	add	esi, 12
	
	inc	DWORD [ebp - 8]
	
	mov	ebx, DWORD [ebp + 12]
	cmp	DWORD [ebp - 8], ebx
	jb	@b
	
	mov	eax, [ebp - 4]
	
	mov	esp, ebp
	pop	ebp
	ret	12

	; Displays a mesh
	; Arguments:
	; - mesh address
DisplayMesh:
	push	ebp
	mov	ebp, esp
	
	mov	ebx, DWORD [ebp + 8]
	invoke	glBindVertexArray, [ebx]
	
	invoke	glUniformMatrix4fv, [MatrixID], 1, GL_TRUE, mvp
	
	invoke	glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, [ebx + 8]
	
	mov	eax, DWORD [ebx + 20]
	mov	ecx, 3
	mul	ecx
	
	invoke	glDrawElements, GL_TRIANGLES, eax, GL_UNSIGNED_INT, NULL
	
	mov	esp, ebp
	pop	ebp
	ret	4

	; Frees the buffers associated with a mesh
	; Arguments:
	; - mesh address
CleanupMesh:
	push	ebp
	mov	ebp, esp
	
	mov	ebx, DWORD [ebp + 8]
	invoke	glDeleteVertexArrays, 1, [ebx]
	invoke	glDeleteBuffers, 1, [ebx + 4]
	invoke	glDeleteBuffers, 1, [ebx + 8]
	invoke	glDeleteBuffers, 1, [ebx + 12]
	
	mov	esp, ebp
	pop	ebp
	ret	4

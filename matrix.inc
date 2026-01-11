	; Computes the matrix product of A and B and places the result in C
	; A and B must be 4 * 4 matrixes in column-major representation
	; Arguments:
	; - A, B, C
MatrixProduct:
	push	ebp
	mov	ebp, esp
	
	mov	ecx, 0
	
      .loop_on_y:
	
	push	ecx
	
	mov	ecx, 0
      .loop_on_x:
	
	push	ecx
	
	mov	ebx, DWORD [ebp + 16]
	
	mov	eax, DWORD [esp + 4] ; contains y
	shl	eax, 2 * 2
	add	ebx, eax
	
	shl	ecx, 2
	add	ebx, ecx ; contains x
	
	mov	DWORD [ebx], 0.0 ; C[y][x] = 0.0
	
	mov	esi, DWORD [ebp + 8] ; A
	add	esi, eax ; A[y][0]
	
	mov	edi, DWORD [ebp + 12] ; B
	add	edi, ecx ; B[0][x]
	
	mov	ecx, 0
      .loop_on_k:
	
	fld	DWORD [esi]
	fmul	DWORD [edi]
	
	fadd	DWORD [ebx]
	
	fstp	DWORD [ebx] ; Could be optimized to keep the result in ST0
	
	add	esi, 4
	add	edi, 16
	
	inc	ecx
	cmp	ecx, 4
	jne	.loop_on_k
	
      .k_end:
	
	pop	ecx
	inc	ecx
	cmp	ecx, 4
	jne	.loop_on_x
	
      .x_end:
	
	
	pop	ecx
	inc	ecx
	cmp	ecx, 4
	jne	.loop_on_y
	
      .end:
	
	mov	esp, ebp
	pop	ebp
	ret	12

	; Computes cot(theta / 2)
	; Arguments:
	; - theta
CotHalf:
	push	ebp
	mov	ebp, esp
	
	push	2.0
	
	fld	DWORD [ebp + 8]
	fdiv	DWORD [esp]
	fsincos
	
	fstp	DWORD [esp]
	
	fdivr	DWORD [esp]
	
	fstp	DWORD [esp]
	pop	eax
	
	mov	esp, ebp
	pop	ebp
	ret	4


	; Computes the perspective matrix of the given parameters.
	; Arguments:
	; - fovy: field of view y in degrees
	; - aspect: aspect ratio ( x / y )
	; - zNear: near clipping plane
	; - zFar: far clipping plane
	; - A: Matrix address
	; Note: here is the returned matrix:
	;
	; |    f				   |
	; |  ------  0      0            0         |
	; |  aspect   	    		 	   |
	; |					   |
	; |    0     f	    0		 0	   |
	; |					   |
	; |		zFar+zNear  2*zFar*zNear   |
	; |    0     0  ----------  ------------   |
	; |         	zNear-zFar   zNear-zFar	   |
	; |					   |
	; |    0     0	    -1		 0         |
	;
	; With f = cotangent(fovy / 2)
	;
Perspective:
	push	ebp
	mov	ebp, esp
	
	mov	edi, DWORD [ebp + 24]
	mov	eax, 0.0
	mov	ecx, 4 * 4
	
	rep	stosd
	
	; theta *= pi / 180
	fldpi
	fmul	DWORD [ebp + 8]
	
	pushd	180
	fidiv	DWORD [esp]
	fstp	DWORD [ebp + 8]
	
	stdcall	CotHalf, DWORD [ebp + 8]
	mov	ebx, DWORD [ebp + 24]
	
	mov	DWORD [ebx + 5 * 4], eax
	
	pushd	eax
	fld	DWORD [esp]
	
	fdiv	DWORD [ebp + 12]
	fstp	DWORD [ebx]
	
	fld	DWORD [ebp + 16]
	fadd	DWORD [ebp + 20]
	
	fld	DWORD [ebp + 16]
	fsub	DWORD [ebp + 20]
	
	fdivp	st1, st0
	
	fstp	DWORD [ebx + 10 * 4]
	
	fld	DWORD [ebp + 16]
	fmul	DWORD [ebp + 20]
	
	fld	DWORD [ebp + 16]
	fsub	DWORD [ebp + 20]
	
	fdivp	st1, st0
	
	pushd	2
	fimul	DWORD [esp]
	
	fstp	DWORD [ebx + 11 * 4]
	
	mov	DWORD [ebx + 14 * 4], -1.0
	
	mov	esp, ebp
	pop	ebp
	ret	20

	; Cross product of two vectors
	; Arguments:
	; - u x, y, z
	; - v x, y, z
	; - w: the destination vector
	; Note: w = (
	; 	uy * vz - uz * vy,
	; 	uz * vx - ux * vz,
	; 	ux * vy - uy * vx
	; )
vec3Cross:
	push	ebp
	mov	ebp, esp
	
	mov	edi, DWORD [ebp + 32]
	
	; x
	fld	DWORD [ebp + 12]
	fmul	DWORD [ebp + 28]
	fld	DWORD [ebp + 16]
	fmul	DWORD [ebp + 24]
	fsubp	st1, st0
	
	fstp	DWORD [edi]
	
	; y
	fld	DWORD [ebp + 16]
	fmul	DWORD [ebp + 20]
	fld	DWORD [ebp + 8]
	fmul	DWORD [ebp + 28]
	fsubp	st1, st0
	
	fstp	DWORD [edi + 4]
	
	; z
	fld	DWORD [ebp + 8]
	fmul	DWORD [ebp + 24]
	fld	DWORD [ebp + 12]
	fmul	DWORD [ebp + 20]
	fsubp	st1, st0
	
	fstp	DWORD [edi + 8]
	
	mov	esp, ebp
	pop	ebp
	ret	28

	; Returns the norm of a vector
	; Arguments:
	; - u x, y, z
Norm:
	push	ebp
	mov	ebp, esp
	
	mov	ecx, 8
	
	fldz
      @@:
	fld	DWORD [ebp + ecx]
	fld	DWORD [ebp + ecx]
	
	fmulp	st1, st0
	faddp	st1, st0
	
	add	ecx, 4
	cmp	ecx, 16
	jbe	@b
	
      @@:
	
	fsqrt
	
	sub	esp, 4
	fstp	DWORD [esp]
	pop	eax
	
	mov	esp, ebp
	pop	ebp
	ret	12

	; Multiplies a vector by a scalar: v = k * u
	; Arguments:
	; - u x, y, z
	; - k
	; - v: The destination vector
vec3Scale:
	push	ebp
	mov	ebp, esp
	
	mov	ecx, 0
	mov	edi, DWORD [ebp + 24]
	
      @@:
	fld	DWORD [ebp + ecx + 8]
	fmul	DWORD [ebp + 20]
	fstp	DWORD [edi + ecx]
	
	add	ecx, 4
	cmp	ecx, 8
	jbe	@b
	
	mov	esp, ebp
	pop	ebp
	ret	20

	; Normalizes a vector
	; Arguments:
	; - u x, y, z
	; - v: The destination vector
Normalize:
	push	ebp
	mov	ebp, esp
	
	stdcall	Norm, DWORD [ebp + 8], DWORD [ebp + 12], DWORD [ebp + 16]
	push	eax
	
	fld1
	fdiv	DWORD [ebp - 4]
	fstp	DWORD [ebp - 4]
	
	stdcall	vec3Scale,\
		DWORD [ebp + 8], DWORD [ebp + 12], DWORD [ebp + 16],\
		DWORD [ebp - 4], DWORD [ebp + 20]
	
	mov	esp, ebp
	pop	ebp
	ret	16

	; Computes the dot product of two vec3
	; Arguments:
	; - u x, y, z
	; - v x, y, z
vec3Dot:
	push	ebp
	mov	ebp, esp
	
	mov	ecx, 8
	
	fldz
      @@:
	fld	DWORD [ebp + ecx]
	fmul	DWORD [ebp + ecx + 12]
	faddp	st1, st0
	
	add	ecx, 4
	cmp	ecx, 16
	jbe	@b
	
	sub	esp, 4
	fstp	DWORD [esp]
	pop	eax
	
	mov	esp, ebp
	pop	ebp
	ret	24

	; Adds a vector with a second vector multiplied by a scalar:
	; w = u + k * v
	; Arguments:
	; - u x, y, z
	; - v x, y, z
	; - k
	; - w
AddMulVec3:
	push	ebp
	mov	ebp, esp
	
	mov	ebx, DWORD [ebp + 36]
	
	fld	DWORD [ebp + 28]
	fmul	DWORD [ebp + 32]
	
	fld	DWORD [ebp + 24]
	fmul	DWORD [ebp + 32]
	
	fld	DWORD [ebp + 20]
	fmul	DWORD [ebp + 32]
	
	fld	DWORD [ebp + 8]
	faddp	st1, st0
	fstp	DWORD [ebx]
	
	fld	DWORD [ebp + 12]
	faddp	st1, st0
	fstp	DWORD [ebx + 4]
	
	fld	DWORD [ebp + 16]
	faddp	st1, st0
	fstp	DWORD [ebx + 8]
	
	mov	esp, ebp
	pop	ebp
	ret	32

	; Equivalent to gluLookAt or glm::lookAt. Look at their specs.
	; Here is where I found the formula for the result:
	; https://rgbguy.github.io/graphics/2025/04/09/DerivingLookAt.html
	;
	; Arguments:
	; - eye x, y, z
	; - target x, y, z
	; - up x, y, z
	; - A: the destination matrix
LookAt:
	push	ebp
	mov	ebp, esp
	
	mov	edi, DWORD [ebp + 44]
	mov	eax, 0.0
	mov	ecx, 4 * 4
	
	rep	stosd
	
	; Store F = target - eye
	mov	edi, DWORD [ebp + 44]
	add	edi, 8 * 4
	
	stdcall	AddMulVec3,\
		DWORD [ebp + 20], DWORD [ebp + 24], DWORD [ebp + 28],\
		DWORD [ebp +  8], DWORD [ebp + 12], DWORD [ebp + 16],\
		-1.0, edi
	
	; Normalize F to obtain f
	mov	esi, DWORD [ebp + 44]
	add	esi, 8 * 4
	stdcall	Normalize, DWORD [esi], DWORD [esi + 4], DWORD [esi + 8], esi
	
	; R = f x up
	mov	eax, DWORD [ebp + 44]
	add	eax, 8 * 4
	
	lea	ebx, [ebp + 32]
	
	mov	esi, DWORD [ebp + 44]
	
	stdcall	vec3Cross,\
		[eax], [eax + 4], [eax + 8],\
		[ebx], [ebx + 4], [ebx + 8], esi
	
	; Normalize R to get r
	mov	esi, DWORD [ebp + 44]
	stdcall	Normalize, DWORD [esi], DWORD [esi + 4], DWORD [esi + 8], esi
	
	; u = r x f
	mov	eax, DWORD [ebp + 44]
	
	mov	ebx, DWORD [ebp + 44]
	add	ebx, 8 * 4
	
	mov	esi, DWORD [ebp + 44]
	add	esi, 4 * 4
	
	stdcall	vec3Cross,\
		[eax], [eax + 4], [eax + 8],\
		[ebx], [ebx + 4], [ebx + 8], esi
	
	; -dot(r, eye)
	mov	eax, DWORD [ebp + 44]
	lea	ebx, [ebp + 8]
	
	stdcall	vec3Dot,\
		[eax], [eax + 4], [eax + 8],\
		[ebx], [ebx + 4], [ebx + 8]
	
	push	eax
	
	fld	DWORD [esp]
	fchs
	fstp	DWORD [esp]
	
	pop	eax
	
	mov	edi, DWORD [ebp + 44]
	mov	DWORD [edi + 3 * 4], eax
	
	; -dot(u, eye)
	mov	eax, DWORD [ebp + 44]
	add	eax, 4 * 4
	lea	ebx, [ebp + 8]
	stdcall	vec3Dot,\
		[eax], [eax + 4], [eax + 8],\
		[ebx], [ebx + 4], [ebx + 8]
	
	push	eax
	
	fld	DWORD [esp]
	fchs
	fstp	DWORD [esp]
	
	pop	eax
	
	mov	edi, DWORD [ebp + 44]
	mov	DWORD [edi + 7 * 4], eax
	
	; dot(f, eye)
	mov	eax, DWORD [ebp + 44]
	add	eax, 8 * 4
	lea	ebx, [ebp + 8]
	stdcall	vec3Dot,\
		[eax], [eax + 4], [eax + 8],\
		[ebx], [ebx + 4], [ebx + 8]
	
	mov	edi, DWORD [ebp + 44]
	mov	DWORD [edi + 11 * 4], eax
	
	; f *= -1
	mov	edi, DWORD [ebp + 44]
	add	edi, 8 * 4
	
	stdcall	vec3Scale, [edi], [edi + 4], [edi + 8], -1.0, edi
	
	mov	edi, DWORD [ebp + 44]
	add	edi, 15 * 4
	mov	DWORD [edi], 1.0
	
	mov	esp, ebp
	pop	ebp
	ret	40

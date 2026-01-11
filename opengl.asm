format PE GUI 4.0
entry start

include 'win32a.inc'

include 'glapi\opengl_const.inc'

REDRAW_FREQ = 20

INITIAL_WIDTH = 432
INITIAL_HEIGHT = 432

MOUSE_SPEED equ 0.0003
PLAYER_SPEED equ 0.003

macro testError {
	invoke	glGetError
	
	test	eax, eax
	jz	@f
	
	invoke	MessageBox, 0, _error, _error, 16
	invoke	ExitProcess, -1
	
	@@:
}

section '.text' code readable executable

  include 'glapi\opengl_procload_text.inc'

  start:
	
	invoke	GetProcessHeap
	mov	[hHeap], eax
	
	invoke	GetModuleHandle, 0
	mov	[wc.hInstance], eax
	invoke	LoadIcon,0, IDI_APPLICATION
	mov	[wc.hIcon], eax
	invoke	LoadCursor, 0, IDC_ARROW
	mov	[wc.hCursor], eax
	invoke	RegisterClass, wc
	invoke	CreateWindowEx,\
			0,\
			_class,\
			_title,\
			WS_VISIBLE+WS_OVERLAPPEDWINDOW+WS_CLIPCHILDREN+WS_CLIPSIBLINGS,\
			16,\
			16,\
			INITIAL_WIDTH,\
			INITIAL_HEIGHT,\
			NULL,\
			NULL,\
			[wc.hInstance],\
			NULL
	mov	[hwnd], eax
	
	mov	[wndWidthHalf], INITIAL_WIDTH/2
	mov	[wndHeightHalf], INITIAL_HEIGHT/2

  msg_loop:
	invoke	GetMessage, msg, NULL, 0, 0
	or	eax, eax
	jz	end_loop
	invoke	TranslateMessage, msg
	invoke	DispatchMessage, msg
	jmp	msg_loop

  end_loop:
	invoke	ExitProcess, [msg.wParam]

	; Loads a BMP file in memory.
	; Arguments:
	; - imagePath ( char * )
LoadBMP:
	push	ebp
	mov	ebp, esp
	
	stdcall	OpenTextFile, DWORD [ebp + 8]
	
	test	eax, eax
	jnz	@f
	
	invoke	MessageBox, 0, _file_error, _error, 16
	invoke	ExitProcess, -1
	
      @@:
	
	cmp	BYTE [eax], 'B'
	jne	.header_error
	
	cmp	BYTE [eax + 1], 'M'
	je	@f
	
      .header_error:
	invoke	MessageBox, 0, _bmp_error, _error, 16
	invoke	ExitProcess, -1
	
      @@:
	mov	ebx, DWORD [eax + 0x0a]
	
	cmp	ebx, 0
	jne	.header_size_ok
	
	mov	ebx, 54
	
      .header_size_ok:
	
	push	ebx
	
	mov	ebx, DWORD [eax + 0x22]
	mov	DWORD [image_size], ebx
	
	mov	ebx, DWORD [eax + 0x12]
	mov	DWORD [bmp_width], ebx
	
	mov	ebx, DWORD [eax + 0x16]
	mov	DWORD [bmp_height], ebx
	
	cmp	DWORD [image_size], 0
	jne	.image_size_ok
	
	push	eax ; Save the pointer to the data
	
	mov	eax, DWORD [bmp_width]
	mov	ebx, DWORD [bmp_height]
	mul	ebx
	
	mov	ebx, 3
	mul	ebx
	
	mov	DWORD [image_size], eax
	
	pop	eax
	
      .image_size_ok:
	
	pop	ebx
	
	lea	esi, [eax + ebx] ; data starts after 54 bytes
	mov	DWORD [hTexData], esi
	
	mov	esp, ebp
	pop	ebp
	ret	4

	; Returns 0 if the file does not exist
	; Arguments: file_name
FileExists:
	push	ebp
	mov	ebp, esp

	invoke	GetFileAttributes, DWORD [ebp + 8]

	mov	ebx, 0
	cmp	eax, -1 ; INVALID_FILE_ATTRIBUTES
	setne	bl
	and	eax, FILE_ATTRIBUTE_DIRECTORY
	not	eax
	and	eax, ebx

	mov	esp, ebp
	pop	ebp

	ret	4

	; Opens a file, returns 0 on error.
	; Arguments: file_name
OpenTextFile:
	push	ebp
	mov	ebp, esp
	
	stdcall	FileExists, DWORD [ebp + 8]
	cmp	eax, FALSE
	je	.error
	
	invoke	CreateFile, DWORD [ebp + 8], GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL
	test	eax, eax
	jz	.error
	push	eax
	
	invoke	GetFileSize, eax, NULL
	mov	ebx, eax
	inc	eax ; For the null terminator
	invoke	HeapAlloc, [hHeap], HEAP_ZERO_MEMORY, eax
	push	eax
	
	invoke	ReadFile, DWORD [ebp - 4], eax, ebx, NULL, NULL
	test	eax, eax
	jz	.error
	
	invoke	CloseHandle, DWORD [ebp - 4]
	
	pop	eax
	jmp	.end

      .error:
	xor	eax, eax

      .end:
	mov	esp, ebp
	pop	ebp
	ret	4

	; Computes the view, projection and mvp matrixes according to
	; the current state
ComputeMatrixes:
	push	ebp
	mov	ebp, esp
	
	fld	DWORD [wndHeightHalf]
	fdiv	DWORD [wndWidthHalf]
	
	sub	esp, 4
	fstp	DWORD [esp]
	
	stdcall	Perspective, 45.0, [ebp - 4], 0.1, 100.0, projection
	
	stdcall	AddMulVec3,\
		[player_x], [player_y], [player_z],\
		[dir_x], [dir_y], [dir_z], 1.0, target_x
	stdcall	LookAt,\
		[player_x], [player_y], [player_z],\
		[target_x], [target_y], [target_z],\
		[up_x], [up_y], [up_z], view
	
	stdcall	MatrixProduct, view, model, tmp
	stdcall	MatrixProduct, projection, tmp, mvp
	
	mov	esp, ebp
	pop	ebp
	ret

MovePlayer:
	invoke	GetCursorPos, CursorPoint
	
	; Compute angles
	pushd	MOUSE_SPEED
	
	mov	eax, [wndWidthHalf]
	sub	eax, DWORD [CursorPoint.x]
	
	pushd	eax
	fild	DWORD [esp]
	
	fimul	DWORD [deltatime]
	fmul	DWORD [esp + 4]
	
	fadd	DWORD [horizontal_angle]
	fstp	DWORD [horizontal_angle]
	add	esp, 4
	
	mov	eax, [wndHeightHalf]
	sub	eax, DWORD [CursorPoint.y]
	
	pushd	eax
	fild	DWORD [esp]
	
	fimul	DWORD [deltatime]
	fmul	DWORD [esp + 4]
	
	fadd	DWORD [vertical_angle]
	fstp	DWORD [vertical_angle]
	add	esp, 8
	
	invoke	SetCursorPos, [wndWidthHalf], [wndHeightHalf]
	
	; Limit vertical angle
	pushd	1.5
	fld	DWORD [esp]
	fld	DWORD [vertical_angle]
	
	fcomi	st1
	fcmovnb	st0, st1
	
	fxch
	fchs
	fxch
	
	fcomi	st1
	fcmovb	st0, st1
	
	fstp	DWORD [vertical_angle]
	fstp	DWORD [esp] ; Why do I have to store just to pop ?
	add	esp, 4
	
	; Dir
	fld	DWORD [horizontal_angle]
	fsin
	fld	DWORD [vertical_angle]
	fcos
	fmulp	st1, st0
	fstp	DWORD [dir_x]
	
	fld	DWORD [vertical_angle]
	fsin
	fstp	DWORD [dir_y]
	
	fld	DWORD [horizontal_angle]
	fcos
	fld	DWORD [vertical_angle]
	fcos
	fmulp	st1, st0
	fstp	DWORD [dir_z]
	
	; Right
	; TODO: simplify with trig
	pushd	2.0
	fldpi
	fdiv	DWORD [esp]
	fstp	DWORD [esp]
	
	fld	DWORD [horizontal_angle]
	fsub	DWORD [esp]
	fsin
	fstp	DWORD [right_x]
	
	mov	DWORD [right_y], 0.0
	
	fld	DWORD [horizontal_angle]
	fsub	DWORD [esp]
	fcos
	fstp	DWORD [right_z]
	
	add	esp, 4
	
	stdcall	vec3Cross,\
		[right_x], [right_y], [right_z],\
		[dir_x], [dir_y], [dir_z], up_x
	
	; Position
	
	pushd	PLAYER_SPEED
	fld	DWORD [esp]
	fimul	DWORD [deltatime]
	fstp	DWORD [esp]
	
	cmp	BYTE [up_pressed], 1
	jne	@f
	
	mov	eax, DWORD [esp]
	
	stdcall	AddMulVec3,\
		[player_x], [player_y], [player_z],\
		[dir_x], [dir_y], [dir_z],\
		eax, player_x
	
	@@:
	
	cmp	BYTE [right_pressed], 1
	jne	@f
	
	mov	eax, DWORD [esp]
	
	stdcall	AddMulVec3,\
		[player_x], [player_y], [player_z],\
		[right_x], [right_y], [right_z],\
		eax, player_x
	
	@@:
	
	fld	DWORD [esp]
	fchs
	fstp	DWORD [esp]
	
	cmp	BYTE [down_pressed], 1
	jne	@f
	
	mov	eax, DWORD [esp]
	
	stdcall	AddMulVec3,\
		[player_x], [player_y], [player_z],\
		[dir_x], [dir_y], [dir_z],\
		eax, player_x
	
	@@:
	
	cmp	BYTE [left_pressed], 1
	jne	@f
	
	mov	eax, DWORD [esp]
	
	stdcall	AddMulVec3,\
		[player_x], [player_y], [player_z],\
		[right_x], [right_y], [right_z],\
		eax, player_x
	
	@@:
	
	add	esp, 4
	
	ret

proc WindowProc hwnd,wmsg,wparam,lparam
	push	ebx esi edi
	cmp	[wmsg], WM_CREATE
	je	.wm_create
	cmp	[wmsg], WM_SIZE
	je	.wm_size
	cmp	[wmsg], WM_TIMER
	je	.wm_timer
	cmp	[wmsg], WM_KEYDOWN
	je	.wm_keydown
	cmp	[wmsg], WM_KEYUP
	je	.wm_keyup
	cmp	[wmsg], WM_DESTROY
	je	.wm_destroy
  .defwndproc:
	invoke	DefWindowProc, [hwnd], [wmsg], [wparam], [lparam]
	jmp	.finish
  .wm_create:
	invoke	GetDC, [hwnd]
	mov	[hdc], eax
	
	mov	edi, pfd
	mov	ecx, sizeof.PIXELFORMATDESCRIPTOR shr 2
	xor	eax, eax
	rep	stosd
	
	mov	[pfd.nSize], sizeof.PIXELFORMATDESCRIPTOR
	mov	[pfd.nVersion], 1
	mov	[pfd.dwFlags], PFD_SUPPORT_OPENGL+PFD_DOUBLEBUFFER+PFD_DRAW_TO_WINDOW
	mov	[pfd.iLayerType], PFD_MAIN_PLANE
	mov	[pfd.iPixelType], PFD_TYPE_RGBA
	mov	[pfd.cColorBits], 16
	mov	[pfd.cDepthBits], 16
	mov	[pfd.cAccumBits], 0
	mov	[pfd.cStencilBits], 0
	
	invoke	ChoosePixelFormat, [hdc], pfd
	invoke	SetPixelFormat, [hdc], eax, pfd
	invoke	wglCreateContext, [hdc]
	mov	[hrc], eax
	
	invoke	wglMakeCurrent, [hdc], [hrc]
	invoke	GetClientRect, [hwnd], rc
	invoke	glViewport, 0, 0, [rc.right], [rc.bottom]
	
	call	load_procs
	
	invoke	glEnable, GL_DEPTH_TEST
	invoke	glDepthFunc, GL_LESS
	
	stdcall	OpenTextFile, _vertex_shader
	push	eax
	stdcall	OpenTextFile, _fragment_shader
	push	eax
	
	invoke	glCreateShader, GL_VERTEX_SHADER
	mov	[vert_s], eax
	lea	eax, [esp + 4]
	invoke	glShaderSource, [vert_s], 1, eax, NULL
	invoke	glCompileShader, [vert_s]
	
	invoke	glGetShaderiv, [vert_s], GL_COMPILE_STATUS, success
	cmp	[success], 0
	jne	.vert_compile_ok
	
	invoke	glGetShaderInfoLog, [vert_s], 512, NULL, infoLog
	invoke	MessageBox, 0, _shader_error, _error, 16
	invoke	MessageBox, 0, infoLog, _error, 16
	invoke	ExitProcess, -1
	
      .vert_compile_ok:
	
	invoke	glCreateShader, GL_FRAGMENT_SHADER
	mov	[frag_s], eax
	mov	eax, esp
	invoke	glShaderSource, [frag_s], 1, eax, NULL
	invoke	glCompileShader, [frag_s]
	
	invoke	glGetShaderiv, [frag_s], GL_COMPILE_STATUS, success
	cmp	[success], 0
	jne	.frag_compile_ok
	
	invoke	glGetShaderInfoLog, [frag_s], 512, NULL, infoLog
	invoke	MessageBox, 0, _shader_error, _error, 16
	invoke	MessageBox, 0, infoLog, _error, 16
	invoke	ExitProcess, -1
	
      .frag_compile_ok:
	
	invoke	glCreateProgram
	mov	[prog], eax
	invoke	glAttachShader, [prog], [vert_s]
	invoke	glAttachShader, [prog], [frag_s]
	invoke	glLinkProgram, [prog]
	
	invoke	glGetProgramiv, [prog], GL_LINK_STATUS, success
	cmp	[success], 0
	jne	.link_ok
	
	invoke	glGetProgramInfoLog, [prog], 512, NULL, infoLog
	invoke	MessageBox, 0, _shader_error, _error, 16
	invoke	MessageBox, 0, infoLog, _error, 16
	invoke	ExitProcess, -1
	
      .link_ok:
	
	invoke	glDeleteShader, [vert_s]
	invoke	glDeleteShader, [frag_s]
	
	invoke	HeapFree, [hHeap], 0, DWORD [esp]
	invoke	HeapFree, [hHeap], 0, DWORD [esp + 4]
	add	esp, 8
	
	; Texture Stuff
	stdcall	LoadBMP, _texture_file
	mov	[hTexFileData], eax
	
	invoke	glGenTextures, 1, TextureID
	
	invoke	glBindTexture, GL_TEXTURE_2D, [TextureID]
	
	invoke	glTexImage2D, GL_TEXTURE_2D, 0, GL_RGB, [bmp_width], [bmp_height], 0, GL_BGRA, GL_UNSIGNED_BYTE, [hTexData]
	
	invoke	glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR
	invoke	glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR
	invoke	glGenerateMipmap, GL_TEXTURE_2D
	
	invoke	HeapFree, [hHeap], 0, [hTexFileData]
	
	stdcall	GetUVSphereMesh, -2.7, 0.0, -5.0, 1.0, 10, 10, mesh1
	stdcall	GetUVSphereMesh, 0.7, 3.0, -5.0, 1.0, 10, 10, mesh2
	stdcall	GetUVSphereMesh, 0.0, 3.0, 5.0, 0.8, 10, 10, mesh3
	stdcall	GetCylinderMesh, 0.0, 0.0, -5.0, 3.0, 0.8, 2, 24, mesh4
	
	stdcall	GetCubeMesh, 2.0, 0.0, 0.0, 0.5, mesh5
	
	stdcall	GetIcosphereMesh, 0.0, 0.0, 0.0, 1, mesh6
	
	; View matrix stuff
	invoke	glGetUniformLocation, [prog], _mvp
	mov	[MatrixID], eax
	
	invoke	SetTimer, [hwnd], 1, REDRAW_FREQ, 0
	invoke	GetTickCount
	mov	[clock], eax
	
	; Initialize mouse movement capture
	invoke	SetCursorPos, INITIAL_WIDTH/2, INITIAL_HEIGHT/2
	invoke	ShowCursor, 0
	
	xor	eax, eax
	jmp	.finish
  .wm_size:
	invoke	GetClientRect, [hwnd], rc
	
	mov	eax, [rc.bottom]
	mov	ebx, [rc.right]
	shr	eax, 1
	shr	ebx, 1
	mov	[wndWidthHalf], eax
	mov	[wndHeightHalf], ebx
	
	invoke	SetCursorPos, eax, ebx
	invoke	glViewport, 0, 0, [rc.right], [rc.bottom]
	
	xor	eax, eax
	jmp	.finish
   .wm_timer:
	invoke	GetTickCount
	mov	ebx, eax
	
	sub	eax, [clock]
	mov	[deltatime], eax
	
	mov	[clock], ebx
	
	invoke	glClearColor, 0.02, 0.3, 0.3, 1.0
	invoke	glClear, GL_COLOR_BUFFER_BIT+GL_DEPTH_BUFFER_BIT
	
	stdcall	MovePlayer
	stdcall	ComputeMatrixes
	
	invoke	glUseProgram, [prog]
	
	stdcall	DisplayMesh, mesh1
	stdcall	DisplayMesh, mesh2
	stdcall	DisplayMesh, mesh3
	stdcall	DisplayMesh, mesh4
	stdcall	DisplayMesh, mesh5
	stdcall	DisplayMesh, mesh6
	
	invoke	SwapBuffers, [hdc]
	
	xor	eax, eax
	jmp	.finish
  .wm_keydown:
	cmp	[wparam], VK_LEFT
	je	.left_d
	cmp	[wparam], VK_RIGHT
	je	.right_d
	cmp	[wparam], VK_UP
	je	.up_d
	cmp	[wparam], VK_DOWN
	je	.down_d
	
	cmp	[wparam], VK_ESCAPE
	je	.wm_destroy
	
	jmp	.defwndproc
      .left_d:
	mov	BYTE [left_pressed], 1
	jmp	.defwndproc
      .right_d:
	mov	BYTE [right_pressed], 1
	jmp	.defwndproc
      .up_d:
	mov	BYTE [up_pressed], 1
	jmp	.defwndproc
      .down_d:
	mov	BYTE [down_pressed], 1
	jmp	.defwndproc
  .wm_keyup:
	cmp	[wparam], VK_LEFT
	je	.left_u
	cmp	[wparam], VK_RIGHT
	je	.right_u
	cmp	[wparam], VK_UP
	je	.up_u
	cmp	[wparam], VK_DOWN
	je	.down_u
	
	jmp	.defwndproc
      .left_u:
	mov	BYTE [left_pressed], 0
	jmp	.defwndproc
      .right_u:
	mov	BYTE [right_pressed], 0
	jmp	.defwndproc
      .up_u:
	mov	BYTE [up_pressed], 0
	jmp	.defwndproc
      .down_u:
	mov	BYTE [down_pressed], 0
	jmp	.defwndproc
  .wm_destroy:
	invoke	wglMakeCurrent, 0, 0
	invoke	wglDeleteContext, [hrc]
	invoke	ReleaseDC, [hwnd], [hdc]
	
	stdcall	CleanupMesh, mesh1
	stdcall	CleanupMesh, mesh2
	stdcall	CleanupMesh, mesh3
	stdcall	CleanupMesh, mesh4
	stdcall	CleanupMesh, mesh5
	stdcall	CleanupMesh, mesh6
	invoke	glDeleteProgram, [prog]
	
	invoke	ShowCursor, 1
	
	invoke	PostQuitMessage, 0
	xor	eax, eax
  .finish:
	pop	edi esi ebx
	ret
endp

include 'matrix.inc'
include 'meshes.inc'

section '.data' data readable writeable
	mesh1 MESH
	mesh2 MESH
	mesh3 MESH
	mesh4 MESH
	mesh5 MESH
	mesh6 MESH
	
	_title db 'OpenGL example', 0
	_class db 'FASMOPENGL32', 0
	_vertex_shader db 'vertex_shader.glsl', 0
	_fragment_shader db 'fragment_shader.glsl', 0
	_texture_file db 'fleur.bmp', 0
	
	_error db 'Error', 0
	_shader_error db 'Shader compilation error', 0
	_file_error db 'The requested file could not be opened', 0
	_bmp_error db 'The BMP file is invalid', 0
	
	_mvp db 'MVP', 0
	
	clock dd 0
	deltatime dd 0
	
	left_pressed db 0
	right_pressed db 0
	up_pressed db 0
	down_pressed db 0
	
	player_x dd 0.0
	player_y dd 0.0
	player_z dd 3.0
	
	vertical_angle dd 0.0
	horizontal_angle dd 3.14
	
	dir_x dd 0.0
	dir_y dd 0.0
	dir_z dd 0.0
	
	right_x dd 0.0
	right_y dd 0.0
	right_z dd 0.0
	
	up_x dd 0.0
	up_y dd 0.0
	up_z dd 0.0
	
	; Computed LookAt target vector
	target_x dd 0.0
	target_y dd 0.0
	target_z dd 0.0
	
	vert_s	GLuint ?
	frag_s	GLuint ?
	prog	GLuint ?
	
	success	dd 0
	infoLog	rb 512
	
	wc WNDCLASS 0,WindowProc,0,0,NULL,NULL,NULL,NULL,NULL,_class
	
	hHeap dd ?
	hwnd dd ?
	hdc dd ?
	hrc dd ?
	hTexFileData dd 0
	
	wndWidthHalf  dd 0
	wndHeightHalf dd 0
	
	msg MSG
	rc RECT
	pfd PIXELFORMATDESCRIPTOR
	
	CursorPoint POINT
	
	MatrixID GLuint 0
	TextureID GLuint 0
	
	; Icosphere data
	vertices dd\
	0.8506508,           0.5257311,         0.0,\          ; 0
	0.000000101405476,   0.8506507,        -0.525731,\     ; 1
	0.000000101405476,   0.8506506,         0.525731,\     ; 2
	0.5257309,          -0.00000006267203, -0.85065067,\   ; 3
	0.52573115,         -0.00000006267203,  0.85065067,\   ; 4
	0.8506508,          -0.5257311,         0.0,\          ; 5
	-0.52573115,         0.00000006267203, -0.85065067,\   ; 6
	-0.8506508,          0.5257311,         0.0,\          ; 7
	-0.5257309,          0.00000006267203,  0.85065067,\   ; 8
	-0.000000101405476, -0.8506506,        -0.525731,\     ; 9
	-0.000000101405476, -0.8506507,         0.525731,\     ; 10
	-0.8506508,         -0.5257311,         0.0            ; 11
	
	indices dd\
	 0,  1,  2,\
	 0,  3,  1,\
	 0,  2,  4,\
	 3,  0,  5,\
	 0,  4,  5,\
	 1,  3,  6,\
	 1,  7,  2,\
	 7,  1,  6,\
	 4,  2,  8,\
	 7,  8,  2,\
	 9,  3,  5,\
	 6,  3,  9,\
	 5,  4, 10,\
	 4,  8, 10,\
	 9,  5, 10,\
	 7,  6, 11,\
	 7, 11,  8,\
	11,  6,  9,\
	 8, 11, 10,\
	10, 11,  9
	
	uv_data dd\
	1.0, 1.0,\
	0.0, 0.0,\
	1.0, 1.0,\
	0.0, 0.0,\
	1.0, 0.5,\
	0.0, 0.0,\
	1.0, 1.0,\
	0.0, 0.0,\
	0.5, 1.0,\
	0.0, 0.0,\
	1.0, 1.0,\
	0.0, 0.0
	
	; Cube data
	cube_vertices dd\
	-1.0, -1.0, -1.0,\
	-1.0, -1.0,  1.0,\
	-1.0,  1.0, -1.0,\
	-1.0,  1.0,  1.0,\
	 1.0, -1.0, -1.0,\
	 1.0, -1.0,  1.0,\
	 1.0,  1.0, -1.0,\
	 1.0,  1.0,  1.0
	
	cube_indices dd\
	0, 1, 2,	1, 3, 2,\
	0, 2, 6,	0, 6, 4,\
	0, 1, 5,	0, 5, 4,\
	4, 6, 7,	4, 7, 5,\
	2, 3, 7,	2, 7, 6,\
	1, 5, 7,	1, 7, 3
	
	cube_uv dd\
	0.0, 1.0,\
	1.0, 0.0,\
	0.5, 0.0,\
	0.0, 0.3,\
	0.0, 0.5,\
	0.3, 0.0,\
	0.0, 0.4,\
	0.4, 0.0

	tmp dd\
	0.0, 0.0, 0.0, 0.0,\
	0.0, 0.0, 0.0, 0.0,\
	0.0, 0.0, 0.0, 0.0,\
	0.0, 0.0, 0.0, 0.0
	
	projection dd\
	0.0, 0.0, 0.0, 0.0,\
	0.0, 0.0, 0.0, 0.0,\
	0.0, 0.0, 0.0, 0.0,\
	0.0, 0.0, 0.0, 0.0
	
	view dd\
	0.0, 0.0, 0.0, 0.0,\
	0.0, 0.0, 0.0, 0.0,\
	0.0, 0.0, 0.0, 0.0,\
	0.0, 0.0, 0.0, 0.0
	
	model dd\
	1.0, 0.0, 0.0, 0.0,\
	0.0, 1.0, 0.0, 0.0,\
	0.0, 0.0, 1.0, 0.0,\
	0.0, 0.0, 0.0, 1.0
	
	mvp dd\
	0.0, 0.0, 0.0, 0.0,\
	0.0, 0.0, 0.0, 0.0,\
	0.0, 0.0, 0.0, 0.0,\
	0.0, 0.0, 0.0, 0.0
	
	; BMP loader data
	bmp_width dd 0
	bmp_height dd 0
	image_size dd 0
	hTexData dd 0
	
	include 'glapi\opengl_procload_data.inc'

section '.idata' import data readable writeable

  library kernel32,'KERNEL32.DLL',\
	  user32,'USER32.DLL',\
	  gdi32,'GDI32.DLL',\
	  opengl,'OPENGL32.DLL',\
	  glu,'GLU32.DLL'

  include 'api\kernel32.inc'
  include 'api\user32.inc'
  include 'api\gdi32.inc'
  
  include 'glapi\opengl_api.inc'

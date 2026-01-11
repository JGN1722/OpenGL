
macro loadgl name {
	invoke  wglGetProcAddress, _#name
	mov     [name], eax
}

load_procs:
	push	ebp
	mov	ebp, esp
	
	loadgl	glGenVertexArrays
	loadgl	glGenBuffers
	loadgl	glBindVertexArray
	loadgl	glBindBuffer
	loadgl	glBufferData
	loadgl	glVertexAttribPointer
	loadgl	glEnableVertexAttribArray
	loadgl	glCreateShader
	loadgl	glShaderSource
	loadgl	glCompileShader
	loadgl	glCreateProgram
	loadgl	glAttachShader
	loadgl	glLinkProgram
	loadgl	glDeleteShader
	loadgl	glUseProgram
	loadgl	glGetShaderiv
	loadgl	glGetShaderInfoLog
	loadgl	glGetProgramiv
	loadgl	glGetProgramInfoLog
	loadgl	glDeleteVertexArrays
	loadgl	glDeleteBuffers
	loadgl	glDeleteProgram
	loadgl	glGetUniformLocation
	loadgl	glUniformMatrix4fv
	loadgl	glGenerateMipmap
	loadgl	glMapBufferRange
	loadgl	glUnmapBuffer
	
	mov	esp, ebp
	pop	ebp
	ret

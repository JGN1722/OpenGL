
macro storegl name {
	_#name db `name, 0
	name dd ?
}

storegl	glGenVertexArrays
storegl	glGenBuffers
storegl	glBindVertexArray
storegl	glBindBuffer
storegl	glBufferData
storegl glVertexAttribPointer
storegl	glEnableVertexAttribArray
storegl	glCreateShader
storegl	glShaderSource
storegl	glCompileShader
storegl	glCreateProgram
storegl	glAttachShader
storegl	glLinkProgram
storegl	glDeleteShader
storegl	glUseProgram
storegl	glGetShaderiv
storegl	glGetShaderInfoLog
storegl	glGetProgramiv
storegl	glGetProgramInfoLog
storegl	glDeleteVertexArrays
storegl	glDeleteBuffers
storegl	glDeleteProgram
storegl	glGetUniformLocation
storegl	glUniformMatrix4fv
storegl	glGenerateMipmap
storegl	glMapBufferRange
storegl	glUnmapBuffer

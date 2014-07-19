void Program::var(GLint name, GLint value)
{
	if (id)
	{
		GL_CHECK(glProgramUniform1i(id, name, value));
	}
}

void Program::var(GLint name, GLuint value)
{
	if (id)
	{
		GL_CHECK(glProgramUniform1ui(id, name, value));
	}
}

void Program::var(GLint name, GLfloat value)
{
	if (id)
	{
		GL_CHECK(glProgramUniform1f(id, name, value));
	}
}

void Program::var(GLint name, GLfloat x, GLfloat y)
{
	if (id)
	{
		GL_CHECK(glProgramUniform2f(id, name, x, y));
	}
}

void Program::var(GLint name, GLfloat x, GLfloat y, GLfloat z)
{
	if (id)
	{
		GL_CHECK(glProgramUniform3f(id, name, x, y, z));
	}
}

void Program::var(GLint name, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	if (id)
	{
		GL_CHECK(glProgramUniform4f(id, name, x, y, z, w));
	}
}

void Program::var(GLint name, const glm::vec2 &value)
{
	if (id)
	{
		GL_CHECK(glProgramUniform2fv(id, name, 1, glm::value_ptr(value)));
	}
}

void Program::var(GLint name, const glm::vec3 &value)
{
	if (id)
	{
		GL_CHECK(glProgramUniform3fv(id, name, 1, glm::value_ptr(value)));
	}
}

void Program::var(GLint name, const glm::vec4 &value)
{
	if (id)
	{
		GL_CHECK(glProgramUniform4fv(id, name, 1, glm::value_ptr(value)));
	}
}

void Program::var(GLint name, const glm::mat3 &value)
{
	if (id)
	{
		GL_CHECK(glProgramUniformMatrix3fv(id, name, 1, GL_FALSE, glm::value_ptr(value)));
	}
}

void Program::var(GLint name, const glm::mat4 &value)
{
	if (id)
	{
		GL_CHECK(glProgramUniformMatrix4fv(id, name, 1, GL_FALSE, glm::value_ptr(value)));
	}
}


GLint Program::var(const std::string &name, GLint value)
{
	GLint location = getName(name);
	var(location, value);
	return location;
}

GLint Program::var(const std::string &name, GLuint value)
{
	GLint location = getName(name);
	var(location, value);
	return location;
}

GLint Program::var(const std::string &name, GLfloat value)
{
	GLint location = getName(name);
	var(location, value);
	return location;
}

GLint Program::var(const std::string &name, GLfloat x, GLfloat y)
{
	GLint location = getName(name);
	var(location, x, y);
	return location;
}

GLint Program::var(const std::string &name, GLfloat x, GLfloat y, GLfloat z)
{
	GLint location = getName(name);
	var(location, x, y, z);
	return location;
}

GLint Program::var(const std::string &name, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	GLint location = getName(name);
	var(location, x, y, z, w);
	return location;
}

GLint Program::var(const std::string &name, const glm::vec2 &value)
{
	GLint location = getName(name);
	var(location, value);
	return location;
}

GLint Program::var(const std::string &name, const glm::vec3 &value)
{
	GLint location = getName(name);
	var(location, value);
	return location;
}

GLint Program::var(const std::string &name, const glm::vec4 &value)
{
	GLint location = getName(name);
	var(location, value);
	return location;
}

GLint Program::var(const std::string &name, const glm::mat3 &value)
{
	GLint location = getName(name);
	var(location, value);
	return location;
}

GLint Program::var(const std::string &name, const glm::mat4 &value)
{
	GLint location = getName(name);
	var(location, value);
	return location;
}

GLint Program::uniform(const string &name, GLint value)
{
	UniformValue &uniformValue = getValue(name);
	uniformValue.type = UniformValue::Type::uniform_int;
	uniformValue.i = value;

	var(uniformValue.id, value);

	return uniformValue.id;
}

GLint Program::uniform(const string &name, GLuint value)
{
	UniformValue &uniformValue = getValue(name);
	uniformValue.type = UniformValue::Type::uniform_uint;
	uniformValue.ui = value;

	var(uniformValue.id, value);

	return uniformValue.id;
}

GLint Program::uniform(const string &name, GLfloat value)
{
	UniformValue &uniformValue = getValue(name);
	uniformValue.type = UniformValue::Type::uniform_float;
	uniformValue.f = value;

	var(uniformValue.id, value);

	return uniformValue.id;
}

GLint Program::uniform(const string &name, GLfloat x, GLfloat y)
{
	UniformValue &uniformValue = getValue(name);
	uniformValue.type = UniformValue::Type::uniform_vec2;
	uniformValue.v2.x = x;
	uniformValue.v2.y = y;

	var(uniformValue.id, uniformValue.v2);

	return uniformValue.id;
}

GLint Program::uniform(const string &name, GLfloat x, GLfloat y, GLfloat z)
{
	UniformValue &uniformValue = getValue(name);
	uniformValue.type = UniformValue::Type::uniform_vec3;
	uniformValue.v3.x = x;
	uniformValue.v3.y = y;
	uniformValue.v3.z = z;

	var(uniformValue.id, uniformValue.v3);

	return uniformValue.id;
}

GLint Program::uniform(const string &name, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	UniformValue &uniformValue = getValue(name);
	uniformValue.type = UniformValue::Type::uniform_vec4;
	uniformValue.v4.x = x;
	uniformValue.v4.y = y;
	uniformValue.v4.z = z;
	uniformValue.v4.w = w;

	var(uniformValue.id, uniformValue.v4);

	return uniformValue.id;
}

GLint Program::uniform(const string &name, const glm::vec2 &value)
{
	UniformValue &uniformValue = getValue(name);
	uniformValue.type = UniformValue::Type::uniform_vec2;
	uniformValue.v2 = value;

	var(uniformValue.id, value);

	return uniformValue.id;
}

GLint Program::uniform(const string &name, const glm::vec3 &value)
{
	UniformValue &uniformValue = getValue(name);
	uniformValue.type = UniformValue::Type::uniform_vec3;
	uniformValue.v3 = value;

	var(uniformValue.id, value);

	return uniformValue.id;
}

GLint Program::uniform(const string &name, const glm::vec4 &value)
{
	UniformValue &uniformValue = getValue(name);
	uniformValue.type = UniformValue::Type::uniform_vec4;
	uniformValue.v4 = value;

	var(uniformValue.id, value);

	return uniformValue.id;
}

GLint Program::uniform(const string &name, const glm::mat3 &value)
{
	UniformValue &uniformValue = getValue(name);
	uniformValue.type = UniformValue::Type::uniform_mat3;
	uniformValue.m3 = value;

	var(uniformValue.id, value);

	return uniformValue.id;
}

GLint Program::uniform(const string &name, const glm::mat4 &value)
{
	UniformValue &uniformValue = getValue(name);
	uniformValue.type = UniformValue::Type::uniform_mat4;
	uniformValue.m4 = value;

	var(uniformValue.id, value);

	return uniformValue.id;
}
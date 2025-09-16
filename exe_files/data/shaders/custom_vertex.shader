void Vertex_Func(in vec3 _positionLocal, in vec3 _texcoord, in vec3 _normal, in int _instanceID, in Instance _instance, in Material material, in float _use2D, out vec4 _glPosition)
{
	Vertex_Transform(_positionLocal, _instance, _use2D, _glPosition);
}
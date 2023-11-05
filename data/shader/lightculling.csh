#include <lightculling.hsh>
#include <blenderlib/intersect_lib.glslh>


layout(local_size_x = 8, local_size_y = 8) in;

shared uint visiblePointLightCount;
shared uint visibleSpotLightCount;

shared IsectFrustum tileFrustum;

shared float depthNear;
shared float depthFar;


void main()
{
	ivec2 pointIndex = ivec2(gl_LocalInvocationID.xy);
	ivec3 clusterID = ivec3(gl_WorkGroupID.xyz);
	ivec3 clusterNumber = ivec3(gl_NumWorkGroups.xyz);

	int clusterIndex = clusterID.z * clusterNumber.y * clusterNumber.x + clusterID.y * clusterNumber.x + clusterID.x;

	const uint clusterOffset = clusterIndex * 4;

	// Initialize shared global values for depth and light count

	if (gl_LocalInvocationIndex == 0)
	{
		visiblePointLightCount = 0;
		visibleSpotLightCount = 0;


		float ZperCluster = (UniformCulling.cameraFar - UniformCulling.cameraNear) / float(clusterNumber.z);
		
		//depthNear = ConvertViewSpaceDepthToDepth(float(clusterID.z) * ZperCluster + UniformCulling.cameraNear, UniformCulling.cameraNear, UniformCulling.cameraFar);
		//depthFar = ConvertViewSpaceDepthToDepth(float(clusterID.z + 1) * ZperCluster + UniformCulling.cameraNear, UniformCulling.cameraNear, UniformCulling.cameraFar);

		float ratio = UniformCulling.cameraFar / UniformCulling.cameraNear;

		depthNear = ConvertViewZToDepth( - UniformCulling.cameraNear * pow(ratio, float(clusterID.z)/ clusterNumber.z));
		depthFar = ConvertViewZToDepth(-UniformCulling.cameraNear * pow(ratio, float(clusterID.z + 1) / clusterNumber.z));

		vec2 uv = vec2(clusterID.xy) / clusterNumber.xy;

		vec2 tileBias = vec2(clusterID.xy) / vec2(clusterNumber.xy);
		vec2 deltaBias = vec2(1.0) / vec2(clusterNumber.xy);  //inverse sampler Dir


		vec3 cornersV[8];
		cornersV[0] = ConvertDepthToViewSpace(depthNear, min(tileBias + vec2(0, deltaBias.y), vec2(1.0f)));
		cornersV[1] = ConvertDepthToViewSpace(depthNear, tileBias + vec2(0, 0));
		cornersV[2] = ConvertDepthToViewSpace(depthFar, tileBias + vec2(0, 0));
		cornersV[3] = ConvertDepthToViewSpace(depthFar, min(tileBias + vec2(0, deltaBias.y), vec2(1.0f)));
		cornersV[4] = ConvertDepthToViewSpace(depthNear, min(tileBias + vec2(deltaBias.x, deltaBias.y), vec2(1.0f)));
		cornersV[5] = ConvertDepthToViewSpace(depthNear, min(tileBias + vec2(deltaBias.x, 0), vec2(1.0f)));
		cornersV[6] = ConvertDepthToViewSpace(depthFar, min(tileBias + vec2(deltaBias.x, 0), vec2(1.0f)));
		cornersV[7] = ConvertDepthToViewSpace(depthFar, min(tileBias + vec2(deltaBias.x, deltaBias.y), vec2(1.0f)));


		tileFrustum = isect_data_setup(shape_frustum(cornersV));
	}

	barrier();


	const uint threadCount = 8 * 8;
	uint passCount = (pointLightUniforms.pointLightCount + threadCount - 1) / threadCount;

	for (uint i = 0; i < passCount; i++)
	{
		// Get the lightIndex to test for this thread / pass. If the index is >= light count, then this thread can stop testing lights
		uint lightIndex = i * threadCount + gl_LocalInvocationIndex;
		if (lightIndex >= pointLightUniforms.pointLightCount)
			break;


		vec3 center =(globalData.vMatrix * vec4(pointLightUniforms.pointLights[lightIndex].location.xyz, 1.0)).xyz;
		float radius = pointLightUniforms.pointLights[lightIndex].radius;

		Sphere sV;
		sV.center = center;
		sV.radius = radius;

		if (intersect(tileFrustum, sV))
		{
			uint offset = atomicAdd(visiblePointLightCount, 1);
			s_VisiblePointLightIndicesBuffer.Indices[clusterOffset + offset] = int(lightIndex);
		}
	}
	barrier();

	if (gl_LocalInvocationIndex == 0)
	{
		s_VisiblePointLightIndicesBuffer.Indices[clusterOffset + visiblePointLightCount] = -1;
	}
}
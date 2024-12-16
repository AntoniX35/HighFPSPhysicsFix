#pragma once

#include <d3d11.h>

class D3D11StateBackupImpl
{
public:
	void Backup(ID3D11DeviceContext* a_ctx, bool a_renderTargets = false)
	{
		if (a_renderTargets)
			a_ctx->OMGetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, RenderTargetViews, &DepthStencilView);

		ScissorRectsCount = ViewportsCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
		a_ctx->RSGetScissorRects(&ScissorRectsCount, ScissorRects);
		a_ctx->RSGetViewports(&ViewportsCount, Viewports);
		a_ctx->RSGetState(&RS);
		a_ctx->OMGetBlendState(&BlendState, BlendFactor, &SampleMask);
		a_ctx->OMGetDepthStencilState(&DepthStencilState, &StencilRef);
		a_ctx->PSGetShaderResources(0, 1, &PSShaderResource);
		a_ctx->PSGetSamplers(0, 1, &PSSampler);
		PSInstancesCount = VSInstancesCount = GSInstancesCount = 256;
		a_ctx->PSGetShader(&PS, PSInstances, &PSInstancesCount);
		a_ctx->VSGetShader(&VS, VSInstances, &VSInstancesCount);
		a_ctx->VSGetConstantBuffers(0, 1, &VSConstantBuffer);
		a_ctx->GSGetShader(&GS, GSInstances, &GSInstancesCount);
		a_ctx->IAGetPrimitiveTopology(&PrimitiveTopology);
		a_ctx->IAGetIndexBuffer(&IndexBuffer, &IndexBufferFormat, &IndexBufferOffset);
		a_ctx->IAGetVertexBuffers(0, 1, &VertexBuffer, &VertexBufferStride, &VertexBufferOffset);
		a_ctx->IAGetInputLayout(&InputLayout);
	}

	void Restore(ID3D11DeviceContext* a_ctx, bool a_renderTargets = false)
	{
		if (a_renderTargets) {
			a_ctx->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, RenderTargetViews, DepthStencilView);

			for (UINT i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; i++) {
				if (RenderTargetViews[i]) {
					RenderTargetViews[i]->Release();
				}
			}

			if (DepthStencilView)
				DepthStencilView->Release();
		}

		a_ctx->RSSetScissorRects(ScissorRectsCount, ScissorRects);
		a_ctx->RSSetViewports(ViewportsCount, Viewports);

		a_ctx->RSSetState(RS);
		if (RS)
			RS->Release();

		a_ctx->OMSetBlendState(BlendState, BlendFactor, SampleMask);
		if (BlendState)
			BlendState->Release();

		a_ctx->OMSetDepthStencilState(DepthStencilState, StencilRef);
		if (DepthStencilState)
			DepthStencilState->Release();

		a_ctx->PSSetShaderResources(0, 1, &PSShaderResource);
		if (PSShaderResource)
			PSShaderResource->Release();

		a_ctx->PSSetSamplers(0, 1, &PSSampler);
		if (PSSampler)
			PSSampler->Release();

		a_ctx->PSSetShader(PS, PSInstances, PSInstancesCount);
		if (PS)
			PS->Release();

		for (UINT i = 0; i < PSInstancesCount; i++)
			if (PSInstances[i])
				PSInstances[i]->Release();

		a_ctx->VSSetShader(VS, VSInstances, VSInstancesCount);
		if (VS)
			VS->Release();

		a_ctx->VSSetConstantBuffers(0, 1, &VSConstantBuffer);
		if (VSConstantBuffer)
			VSConstantBuffer->Release();

		a_ctx->GSSetShader(GS, GSInstances, GSInstancesCount);
		if (GS)
			GS->Release();

		for (UINT i = 0; i < VSInstancesCount; i++)
			if (VSInstances[i])
				VSInstances[i]->Release();

		a_ctx->IASetPrimitiveTopology(PrimitiveTopology);

		a_ctx->IASetIndexBuffer(IndexBuffer, IndexBufferFormat, IndexBufferOffset);
		if (IndexBuffer)
			IndexBuffer->Release();

		a_ctx->IASetVertexBuffers(0, 1, &VertexBuffer, &VertexBufferStride, &VertexBufferOffset);
		if (VertexBuffer)
			VertexBuffer->Release();

		a_ctx->IASetInputLayout(InputLayout);
		if (InputLayout)
			InputLayout->Release();
	}

private:
	UINT                      ScissorRectsCount, ViewportsCount;
	D3D11_RECT                ScissorRects[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
	D3D11_VIEWPORT            Viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
	ID3D11RasterizerState*    RS;
	ID3D11BlendState*         BlendState;
	FLOAT                     BlendFactor[4];
	UINT                      SampleMask;
	UINT                      StencilRef;
	ID3D11DepthStencilState*  DepthStencilState;
	ID3D11ShaderResourceView* PSShaderResource;
	ID3D11SamplerState*       PSSampler;
	ID3D11PixelShader*        PS;
	ID3D11VertexShader*       VS;
	ID3D11GeometryShader*     GS;
	UINT                      PSInstancesCount, VSInstancesCount, GSInstancesCount;
	ID3D11ClassInstance *     PSInstances[256], *VSInstances[256], *GSInstances[256];
	D3D11_PRIMITIVE_TOPOLOGY  PrimitiveTopology;
	ID3D11Buffer *            IndexBuffer, *VertexBuffer, *VSConstantBuffer;
	UINT                      IndexBufferOffset, VertexBufferStride, VertexBufferOffset;
	DXGI_FORMAT               IndexBufferFormat;
	ID3D11InputLayout*        InputLayout;
	ID3D11RenderTargetView*   RenderTargetViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
	ID3D11DepthStencilView*   DepthStencilView;
};

class D3D11StateBackup
{
public:
	D3D11StateBackup() = delete;

	D3D11StateBackup(
		D3D11StateBackupImpl* a_state,
		ID3D11DeviceContext*  a_ctx,
		bool                  a_renderTargets = false) :
		m_state(a_state),
		m_ctx(a_ctx),
		m_renderTargets(a_renderTargets)
	{
		m_state->Backup(a_ctx, a_renderTargets);
	}

	~D3D11StateBackup()
	{
		m_state->Restore(m_ctx, m_renderTargets);
	};

	bool                  m_renderTargets;
	ID3D11DeviceContext*  m_ctx;
	D3D11StateBackupImpl* m_state;
};

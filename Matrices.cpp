//-----------------------------------------------------------------------------
// File: Matrices.cpp
//
// Desc: Now that we know how to create a device and render some 2D vertices,
//       this tutorial goes the next step and renders 3D geometry. To deal with
//       3D geometry we need to introduce the use of 4x4 Matrices to transform
//       the geometry with translations, rotations, scaling, and setting up our
//       camera.
//
//       Geometry is defined in model space. We can move it (translation),
//       rotate it (rotation), or stretch it (scaling) using a world transform.
//       The geometry is then said to be in world space. Next, we need to
//       position the camera, or eye point, somewhere to look at the geometry.
//       Another transform, via the view matrix, is used, to position and
//       rotate our view. With the geometry then in view space, our last
//       transform is the projection transform, which "projects" the 3D scene
//       into our 2D viewport.
//
//       Note that in this tutorial, we are introducing the use of D3DX, which
//       is a set of helper utilities for D3D. In this case, we are using some
//       of D3DX's useful matrix initialization functions. To use D3DX, simply
//       include <d3dx9.h> and link with d3dx9.lib.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include <Windows.h>
#include <mmsystem.h>
#include <d3dx9.h>
#pragma warning( disable : 4996 ) // disable deprecated warning 
#include <strsafe.h>
#pragma warning( default : 4996 )


#define SAFE_RELEASE(A) { if (A != nullptr) A->Release(); A = nullptr;}


//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
LPDIRECT3D9                  g_pD3D = NULL; // Used to create the D3DDevice
LPDIRECT3DDEVICE9            g_pd3dDevice = NULL; // Our rendering device
IDirect3DVertexDeclaration9* g_pDecl = 0;
ID3DXConstantTable*          g_constTable = 0;
ID3DXBuffer*                 g_pixelShaderBuffer = 0;
IDirect3DPixelShader9*       g_pixelShader = 0;
ID3DXBuffer*                 g_vertexShaderBuffer = 0;
IDirect3DVertexShader9*      g_vertexShader = 0;
ID3DXBuffer*                 g_errorBuffer = 0;
IDirect3DVertexBuffer9*      g_vertexBuffer = 0;
IDirect3DIndexBuffer9*       g_indexBuffer = 0;
IDirect3DTexture9*           g_texture = 0;
IDirect3DTexture9*           g_texture1 = 0;
IDirect3DTexture9*           g_texture2 = 0;
IDirect3DSurface9*           g_renderTarget = 0;
IDirect3DSurface9*           g_zBuffer = 0;
IDirect3DSwapChain9* g_swapChain = 0;
IDirect3DSurface9* g_backBuffer = 0;
const static int             WIDTH = 640;
const static int             HEIGHT = 480;


struct Vertex
{
    Vertex() {}
    Vertex(
        float x, float y, float z, float w,
        float u, float v)
    {
        _x = x;  _y = y;  _z = z, _w = w;
        _u = u;  _v = v;
    }
    float _x, _y, _z, _w;
    float _u, _v; // texture coordinates
};
#define FVF_VERTEX (D3DFVF_XYZ | D3DFVF_TEX1)

// A structure for our custom vertex type
struct CUSTOMVERTEX
{
    FLOAT x, y, z;      // The untransformed, 3D position for the vertex
};

// Our custom FVF, which describes our custom vertex structure
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)


//-----------------------------------------------------------------------------
// Name: InitD3D()
// Desc: Initializes Direct3D
//-----------------------------------------------------------------------------
HRESULT InitD3D( HWND hWnd )
{
    // Create the D3D object.
    if( NULL == ( g_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
        return E_FAIL;

    // Set up the structure used to create the D3DDevice
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof( d3dpp ) );
    d3dpp.BackBufferWidth = WIDTH;
    d3dpp.BackBufferHeight = HEIGHT;
    d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
    d3dpp.BackBufferCount = 2;
    d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
    d3dpp.MultiSampleQuality = 0;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.Windowed = TRUE;
    d3dpp.hDeviceWindow = hWnd;
    d3dpp.EnableAutoDepthStencil = true;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
    d3dpp.Flags = 0;
    d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    
    // Create the D3DDevice
    if( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
                                      D3DCREATE_HARDWARE_VERTEXPROCESSING,
                                      &d3dpp, &g_pd3dDevice ) ) )
    {
        LONG rer = g_pd3dDevice->Release();
        return E_FAIL;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InitGeometry()
// Desc: Creates the scene geometry
//-----------------------------------------------------------------------------
HRESULT Init()
{
    HRESULT hr;
    /* Create D3DVertex Declaration */
    D3DVERTEXELEMENT9 decl[] =
    {
        {0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
        {0, 16, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
        D3DDECL_END()
    };
    if (FAILED(g_pd3dDevice->CreateVertexDeclaration(decl, &g_pDecl))) 
    {
        return E_FAIL;
    }
    /* Create D3DVertex Declaration */

    /* Create Pixel Shader */
    hr = D3DXCompileShaderFromFile(L"pixelShader.txt", NULL, NULL, "Main", "ps_2_0", D3DXSHADER_DEBUG, &g_pixelShaderBuffer, &g_errorBuffer, &g_constTable);
    hr = g_pd3dDevice->CreatePixelShader((DWORD*)g_pixelShaderBuffer->GetBufferPointer(), &g_pixelShader);
    /* Create Pixel Shader */

    /* Create Vertex Shader */
    hr = D3DXCompileShaderFromFile(L"vertexShader.txt", NULL, NULL, "Main", "vs_2_0", D3DXSHADER_DEBUG, &g_vertexShaderBuffer, &g_errorBuffer, &g_constTable);
    hr = g_pd3dDevice->CreateVertexShader((DWORD*)g_vertexShaderBuffer->GetBufferPointer(), &g_vertexShader);
    /* Create Vertex Shader */

    /*create Vertex Buffer And Index Buffer*/
    /*
    hr = g_pd3dDevice->CreateIndexBuffer( 36 * sizeof(WORD), D3DUSAGE_DYNAMIC, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &g_indexBuffer, 0);
    */

    //hr = g_pd3dDevice->CreateVertexBuffer( 8 * sizeof(Vertex), D3DUSAGE_WRITEONLY, Vertex::FVF, D3DPOOL_MANAGED, &g_vertexBuffer, 0);
    //D3DPOOL_SYSTEMMEM
    //hr = g_pd3dDevice->CreateVertexBuffer( 8 * sizeof(Vertex), D3DUSAGE_WRITEONLY, Vertex::FVF, D3DPOOL_SYSTEMMEM, &g_vertexBuffer, 0);
    //hr = g_pd3dDevice->CreateIndexBuffer( 36 * sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_SYSTEMMEM, &g_indexBuffer, 0);
    hr = g_pd3dDevice->CreateVertexBuffer(8 * sizeof(Vertex), D3DUSAGE_WRITEONLY, FVF_VERTEX, D3DPOOL_MANAGED, &g_vertexBuffer, 0);
    hr = g_pd3dDevice->CreateIndexBuffer( 36 * sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &g_indexBuffer, 0);
    /*create Vertex Buffer And Index Buffer*/

    hr = g_pd3dDevice->CreateTexture(256, 256, 1, 0/*D3DUSAGE_AUTOGENMIPMAP*/, D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &g_texture1, nullptr);
    hr = D3DXCreateTextureFromFile(g_pd3dDevice, L"crate.jpg", &g_texture);
    
    D3DLOCKED_RECT rect;

    RECT            region;
    region.left = 0;
    region.right = 100;
    region.top = 0;
    region.bottom = 100;
    hr = g_texture->LockRect(0, &rect, 0, 0);
    hr = g_texture->LockRect(0, &rect, 0, 0);
    D3DLOCKED_RECT rect1;
    hr = g_texture1->LockRect(0, &rect1, 0, 0);
    memcpy(rect1.pBits, rect.pBits, 262144);
    hr = g_texture->UnlockRect(0);
    hr = g_texture1->UnlockRect(0);



    hr = g_pd3dDevice->CreateTexture(256, 256, 0, D3DUSAGE_AUTOGENMIPMAP, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &g_texture2, nullptr);
    //g_pd3dDevice->CreateRenderTarget();
    //g_pd3dDevice->CreateDepthStencilSurface();
    hr = g_pd3dDevice->UpdateTexture(g_texture1, g_texture2);

    hr = g_pd3dDevice->SetTexture(0, g_texture2);
    //hr = g_texture1->AddDirtyRect(0);
    //hr = g_pd3dDevice->UpdateTexture(g_texture, g_texture1);
    //hr = g_pd3dDevice->CreateTexture(256, 256, 0, )
    hr = g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    hr = g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    hr = g_pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

    hr = g_pd3dDevice->CreateRenderTarget(WIDTH, HEIGHT, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0, false, &g_renderTarget, 0);
    hr = g_pd3dDevice->SetRenderTarget(0, g_renderTarget);
    hr = g_pd3dDevice->CreateDepthStencilSurface(WIDTH, HEIGHT, D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0, false, &g_zBuffer, 0);
    hr = g_pd3dDevice->SetDepthStencilSurface(g_zBuffer);
    

    hr = g_pd3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &g_backBuffer);
    hr = g_pd3dDevice->GetSwapChain(0, &g_swapChain);
    //D3DXCreateTexture();
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Draws the scene
//-----------------------------------------------------------------------------
VOID Render()
{
    HRESULT hr;

    /* Fill the vertex buffer.  */
    Vertex* vertices;
    hr = g_vertexBuffer->Lock(0, 0, (void**)&vertices, 0);
    hr = g_vertexBuffer->Lock(0, 0, (void**)&vertices, 0);
    // vertices of a unit cube
    vertices[0] = Vertex(-1.0f, -1.0f, -1.0f, 1.0f,0.0f, 0.0f);
    vertices[1] = Vertex(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f);
    vertices[2] = Vertex(1.0f, 1.0f, -1.0f , 1.0f, 1.0f, 1.0f);
    vertices[3] = Vertex(1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f);
    vertices[4] = Vertex(-1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f);
    vertices[5] = Vertex(-1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f);
    vertices[6] = Vertex(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    vertices[7] = Vertex(1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    g_vertexBuffer->Unlock();
    /* Fill the vertex buffer.  */

    
    
    /* Fill the index buffer.  */
    WORD* indices = 0;
    g_indexBuffer->Lock(0, 0, (void**)&indices, 0);

    // front side
    indices[0] = 0; indices[1] = 1; indices[2] = 2;
    indices[3] = 0; indices[4] = 2; indices[5] = 3;

    // back side
    indices[6] = 4; indices[7] = 6; indices[8] = 5;
    indices[9] = 4; indices[10] = 7; indices[11] = 6;

    // left side
    indices[12] = 4; indices[13] = 5; indices[14] = 1;
    indices[15] = 4; indices[16] = 1; indices[17] = 0;

    // right side
    indices[18] = 3; indices[19] = 2; indices[20] = 6;
    indices[21] = 3; indices[22] = 6; indices[23] = 7;

    // top
    indices[24] = 1; indices[25] = 5; indices[26] = 6;
    indices[27] = 1; indices[28] = 6; indices[29] = 2;

    // bottom
    indices[30] = 4; indices[31] = 0; indices[32] = 3;
    indices[33] = 4; indices[34] = 3; indices[35] = 7;

    g_indexBuffer->Unlock();

    /* apply mvp transformational matrix */
    D3DXMATRIX world;
    // incremement y-rotation angle each frame
    static float y = 0.0f;
    D3DXMatrixRotationY(&world, y);
    y += 0.0001;

    // reset angle to zero when angle reaches 2*PI
    if (y >= 6.28f)
        y = 0.0f;

    D3DXVECTOR3 position(0.0f, 0.0f, -5.0f);
    D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
    D3DXMATRIX V;
    D3DXMatrixLookAtLH(&V, &position, &target, &up);


    D3DXMATRIX proj;
    D3DXMatrixPerspectiveFovLH(
        &proj,
        D3DX_PI * 0.5f, // 90 - degree
        (float)WIDTH / (float)HEIGHT,
        1.0f,
        1000.0f);


    hr = g_pd3dDevice->SetVertexShader(g_vertexShader);
    hr = g_pd3dDevice->SetPixelShader(g_pixelShader);

    D3DXHANDLE h = g_constTable->GetConstantByName(0, "mWorldViewProj");
    D3DXMATRIXA16 temp = world * V * proj;
    g_constTable->SetMatrix(g_pd3dDevice, h, (const D3DXMATRIX*)&temp);

    hr = g_pd3dDevice->SetRenderTarget(0, g_renderTarget);
    hr = g_pd3dDevice->SetDepthStencilSurface(g_zBuffer);
    // Clear the backbuffer to a black color
    g_pd3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xfabfffff, 1.0f, 0);

    hr = g_pd3dDevice->SetTexture(0, g_texture2);
    hr = g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    hr = g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    hr = g_pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

    // Begin the scene
    if( SUCCEEDED( g_pd3dDevice->BeginScene() ) )
    {
        hr = g_pd3dDevice->SetStreamSource( 0, g_vertexBuffer, 0, sizeof( Vertex ) );
        hr = g_pd3dDevice->SetIndices( g_indexBuffer );
        hr = g_pd3dDevice->SetVertexDeclaration(g_pDecl);
        g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 8, 0, 12);
        // End the scene
        g_pd3dDevice->EndScene();
    }
    hr = g_pd3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &g_backBuffer);
    hr = g_pd3dDevice->StretchRect(g_renderTarget, 0, g_backBuffer, 0, D3DTEXF_NONE);
    /*
    D3DSURFACE_DESC renderTargetDesc;
    D3DSURFACE_DESC backBufferDesc;
    hr = g_renderTarget->GetDesc(&renderTargetDesc);
    hr = g_backBuffer->GetDesc(&backBufferDesc);
    hr = g_pd3dDevice->UpdateSurface(g_renderTarget, 0, g_backBuffer, 0);
    hr = g_pd3dDevice->GetRenderTargetData(g_renderTarget, g_backBuffer);

    D3DLOCKED_RECT rect;
    hr = g_renderTarget->LockRect(&rect,0, 0);

    g_swapChain->
    
    D3DLOCKED_RECT rect1;
    g_backBuffer->GetPrivateData
    hr = g_backBuffer->LockRect(&rect1,0, 0);
    */
    
    // Present the backbuffer contents to the display
    hr = g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}



//-----------------------------------------------------------------------------
// Name: Cleanup()
// Desc: Releases all previously initialized objects
//-----------------------------------------------------------------------------
VOID Cleanup()
{
    SAFE_RELEASE(g_pDecl);
    SAFE_RELEASE(g_constTable);
    SAFE_RELEASE(g_pixelShaderBuffer);
    SAFE_RELEASE(g_pixelShader);
    SAFE_RELEASE(g_vertexShaderBuffer);
    SAFE_RELEASE(g_vertexShader);
    SAFE_RELEASE(g_errorBuffer);
    SAFE_RELEASE(g_vertexBuffer);
    SAFE_RELEASE(g_texture);
    SAFE_RELEASE(g_texture1);
    SAFE_RELEASE(g_texture2);
    SAFE_RELEASE(g_indexBuffer);
    SAFE_RELEASE(g_pd3dDevice);
    SAFE_RELEASE(g_pD3D);
}



//-----------------------------------------------------------------------------
// Name: SetupMatrices()
// Desc: Sets up the world, view, and projection transform Matrices.
//-----------------------------------------------------------------------------




//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: The window's message handler
//-----------------------------------------------------------------------------
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
        case WM_DESTROY:
            Cleanup();
            PostQuitMessage( 0 );
            return 0;
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: The application's entry point
//-----------------------------------------------------------------------------
INT WINAPI wWinMain( HINSTANCE hInst, HINSTANCE, LPWSTR, INT )
{
    UNREFERENCED_PARAMETER( hInst );

    // Register the window class
    WNDCLASSEX wc =
    {
        sizeof( WNDCLASSEX ), CS_CLASSDC, MsgProc, 0L, 0L,
        GetModuleHandle( NULL ), NULL, NULL, NULL, NULL,
        L"D3D Tutorial", NULL
    };
    RegisterClassEx( &wc );

    // Create the application's window
    HWND hWnd = CreateWindow( L"D3D Tutorial", L"D3D Tutorial 03: Matrices",
                              WS_OVERLAPPEDWINDOW, 100, 100, 640, 480,
                              NULL, NULL, wc.hInstance, NULL );

    // Initialize Direct3D
    if( SUCCEEDED( InitD3D( hWnd ) ) )
    {
        // Create the scene geometry
        if( SUCCEEDED( Init() ) )
        {
            // Show the window
            ShowWindow( hWnd, SW_SHOWDEFAULT );
            UpdateWindow( hWnd );

            // Enter the message loop
            MSG msg;
            ZeroMemory( &msg, sizeof( msg ) );
            while( msg.message != WM_QUIT )
            {
                if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
                {
                    TranslateMessage( &msg );
                    DispatchMessage( &msg );
                }
                else
                    Render();
            }
        }
    }

    UnregisterClass( L"D3D Tutorial", wc.hInstance );
    return 0;
}




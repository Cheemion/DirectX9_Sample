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




//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
LPDIRECT3D9                  g_pD3D = NULL; // Used to create the D3DDevice
LPDIRECT3DDEVICE9            g_pd3dDevice = NULL; // Our rendering device
IDirect3DVertexDeclaration9* g_pDecl = 0;
ID3DXConstantTable*          g_constTable = 0;
ID3DXBuffer*                 g_vertexShaderBuffer = 0;
IDirect3DVertexShader9*      g_vertexShader = 0;
ID3DXBuffer*                 g_errorBuffer = 0;
IDirect3DVertexBuffer9*      g_vertexBuffer = 0;
IDirect3DIndexBuffer9*       g_indexBuffer = 0;
const static int             WIDTH = 640;
const static int             HEIGHT = 480;


struct Vertex
{
    Vertex() {}
    Vertex(float x, float y, float z)
    {
        _x = x;  _y = y;  _z = z;
    }
    float _x, _y, _z;
    static const DWORD FVF;
};
const DWORD Vertex::FVF = D3DFVF_XYZ;


// A structure for our custom vertex type
struct CUSTOMVERTEX
{
    FLOAT x, y, z;      // The untransformed, 3D position for the vertex
};

// Our custom FVF, which describes our custom vertex structure
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)


VOID SetupMatrices();

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
    d3dpp.BackBufferCount = 1;
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
        {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
        D3DDECL_END()
    };
    if (FAILED(g_pd3dDevice->CreateVertexDeclaration(decl, &g_pDecl))) 
    {
        return E_FAIL;
    }
    /* Create D3DVertex Declaration */


    /* Create Vertex Shader */
    hr = D3DXCompileShaderFromFile(L"vertexShader.txt", NULL, NULL, "Main", "vs_2_0", D3DXSHADER_DEBUG, &g_vertexShaderBuffer, &g_errorBuffer, &g_constTable);
    hr = g_pd3dDevice->CreateVertexShader((DWORD*)g_vertexShaderBuffer->GetBufferPointer(), &g_vertexShader);
    /* Create Vertex Shader */

    /*create Vertex Buffer And Index Buffer*/
    hr = g_pd3dDevice->CreateVertexBuffer( 8 * sizeof(Vertex),
									D3DUSAGE_WRITEONLY,
									Vertex::FVF,
									D3DPOOL_MANAGED,
									&g_vertexBuffer,
									0);

    hr = g_pd3dDevice->CreateIndexBuffer(
										36 * sizeof(WORD),
										D3DUSAGE_WRITEONLY,
										D3DFMT_INDEX16,
										D3DPOOL_MANAGED,
										&g_indexBuffer,
										0);

    /*create Vertex Buffer And Index Buffer*/


    




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
    g_vertexBuffer->Lock(0, 0, (void**)&vertices, 0);
    // vertices of a unit cube
    vertices[0] = Vertex(-1.0f, -1.0f, -1.0f);
    vertices[1] = Vertex(-1.0f, 1.0f, -1.0f);
    vertices[2] = Vertex(1.0f, 1.0f, -1.0f);
    vertices[3] = Vertex(1.0f, -1.0f, -1.0f);
    vertices[4] = Vertex(-1.0f, -1.0f, 1.0f);
    vertices[5] = Vertex(-1.0f, 1.0f, 1.0f);
    vertices[6] = Vertex(1.0f, 1.0f, 1.0f);
    vertices[7] = Vertex(1.0f, -1.0f, 1.0f);
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

    D3DXVECTOR3 position(0.0f, 0.0f, -5.0f);
    D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
    D3DXMATRIX V;
    D3DXMatrixLookAtLH(&V, &position, &target, &up);

    g_pd3dDevice->SetTransform(D3DTS_VIEW, &V);

    D3DXMATRIX proj;
    D3DXMatrixPerspectiveFovLH(
        &proj,
        D3DX_PI * 0.5f, // 90 - degree
        (float)WIDTH / (float)HEIGHT,
        1.0f,
        1000.0f);
    g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &proj);

    g_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);


    /* Fill the index buffer.  */

    //hr =  g_pd3dDevice->SetVertexDeclaration(g_pDecl);

    //hr = g_pd3dDevice->SetVertexShader(g_vertexShader);

    // Turn off culling, so we see the front and back of the triangle
    //g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

    // Turn off D3D lighting, since we are providing our own vertex colors
    //g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

    // Clear the backbuffer to a black color
    g_pd3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0);

    // Begin the scene
    if( SUCCEEDED( g_pd3dDevice->BeginScene() ) )
    {
        // Setup the world, view, and projection Matrices
        //SetupMatrices();

        // Render the vertex buffer contents

            //
        // spin the cube:
        //
        D3DXMATRIX Rx, Ry;

        // rotate 45 degrees on x-axis
        D3DXMatrixRotationX(&Rx, 3.14f / 4.0f);

        // incremement y-rotation angle each frame
        static float y = 0.0f;
        D3DXMatrixRotationY(&Ry, y);
        y += 0.0001;

        // reset angle to zero when angle reaches 2*PI
        if (y >= 6.28f)
            y = 0.0f;

        // combine x- and y-axis rotation transformations.
        D3DXMATRIX p = Rx * Ry;

        g_pd3dDevice->SetTransform(D3DTS_WORLD, &p);

        g_pd3dDevice->SetStreamSource( 0, g_vertexBuffer, 0, sizeof( Vertex ) );
        g_pd3dDevice->SetIndices( g_indexBuffer );
        g_pd3dDevice->SetFVF(Vertex::FVF);

        //g_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
		//g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 1 );

        g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 8, 0, 12);

        // End the scene
        g_pd3dDevice->EndScene();
    }

    // Present the backbuffer contents to the display
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}




//-----------------------------------------------------------------------------
// Name: Cleanup()
// Desc: Releases all previously initialized objects
//-----------------------------------------------------------------------------
VOID Cleanup()
{

    if( g_pd3dDevice != NULL )
        g_pd3dDevice->Release();

    if( g_pD3D != NULL )
        g_pD3D->Release();
}



//-----------------------------------------------------------------------------
// Name: SetupMatrices()
// Desc: Sets up the world, view, and projection transform Matrices.
//-----------------------------------------------------------------------------
VOID SetupMatrices()
{
    // For our world matrix, we will just rotate the object about the y-axis.
    D3DXMATRIXA16 matWorld;

    // Set up the rotation matrix to generate 1 full rotation (2*PI radians) 
    // every 1000 ms. To avoid the loss of precision inherent in very high 
    // floating point numbers, the system time is modulated by the rotation 
    // period before conversion to a radian angle.
    UINT iTime = timeGetTime() % 1000;
    FLOAT fAngle = iTime * ( 2.0f * D3DX_PI ) / 1000.0f;
    D3DXMatrixRotationY( &matWorld, fAngle );
    g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

    // Set up our view matrix. A view matrix can be defined given an eye point,
    // a point to lookat, and a direction for which way is up. Here, we set the
    // eye five units back along the z-axis and up three units, look at the
    // origin, and define "up" to be in the y-direction.
    D3DXVECTOR3 vEyePt( 0.0f, 3.0f,-5.0f );
    D3DXVECTOR3 vLookatPt( 0.0f, 0.0f, 0.0f );
    D3DXVECTOR3 vUpVec( 0.0f, 1.0f, 0.0f );
    D3DXMATRIXA16 matView;
    D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );
    g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

    // For the projection matrix, we set up a perspective transform (which
    // transforms geometry from 3D view space to 2D viewport space, with
    // a perspective divide making objects smaller in the distance). To build
    // a perpsective transform, we need the field of view (1/4 pi is common),
    // the aspect ratio, and the near and far clipping planes (which define at
    // what distances geometry should be no longer be rendered).
    D3DXMATRIXA16 matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );


    D3DXHANDLE h = g_constTable->GetConstantByName(0, "mWorldViewProj");
    D3DXMATRIXA16 temp = matWorld* matView* matProj;
    g_constTable->SetMatrix(g_pd3dDevice, h, (const D3DXMATRIX*)&temp);

}






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




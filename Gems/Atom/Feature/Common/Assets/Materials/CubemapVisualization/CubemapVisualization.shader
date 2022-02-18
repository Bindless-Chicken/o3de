{
    "Source" : "./CubemapVisualization.azsl",

    "DepthStencilState" :
    {
        "Depth" :
        {
            "Enable" : true,
            "CompareFunc" : "GreaterEqual"
        },
        "Stencil" :
        {
            "Enable" : false
        }
    },

    "ProgramSettings":
    {
        "EntryPoints":
        [
            {
                "name": "CubemapVisualization_MainVS",
                "type": "Vertex"
            },
            {
                "name": "CubemapVisualization_MainPS",
                "type": "Fragment"
            }
        ]
    },

    "DrawList" : "forward"
}

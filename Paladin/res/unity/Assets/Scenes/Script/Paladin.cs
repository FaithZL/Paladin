using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;

public class Paladin : MonoBehaviour {

    public int threadNum = 0;

    static private String _sign = "-----------";

    [Header("--------filter param--------")]
    public Filter filterName;
    public Vector2 filterRadius = new Vector2(1, 1);
    [Header("Gaussian")]
    public float alpha = 2;
    [Header("Mitchell")]
    public float b = 1.0f / 3.0f;
    public float c = 1.0f / 3.0f;
    [Header("Sinc")]
    public float tau = 3.0f;


    [Header("--------film param--------")]
    public Vector2Int res = new Vector2Int(100,100);
    public Rect cropWindow = new Rect(0,0,1,1);
    public string outputName = "paladin";
    public FileFormat fileFormat;
    [Range(0, 1)]
    public float scale = 1.0f;
    [Range(0, 1)]
    public float diagonal = 1.0f;

    [Header("--------sampler param--------")]
    public Sampler samplerName;
    public int spp = 2;
    public int xSpp = 1;
    public int ySpp = 1;

    [Header("--------integrator param--------")]
    public Integrator integratorName;
    public int maxBounce = 6;
    public float rrThreshold = 1;
    public LightSampleStrategy lightSampleStrategy;

    [Header("--------camera param--------")]
    public float shutterOpen = 0;
    public float shutterClose = 1;
    public float lensRadius = 0;
    public float focalDistance = 100;
    public float fov = 45;

    [Header("--------accelerator--------")]
    public Accelerator acceleratorName;
    public int maxPrimsInNode = 1;
    public AclrtSplitMethod splitMethod;


    private List<MeshFilter> _primitives;

    private List<Light> _lights;

    private Camera _camera;

    private System.Object _output;


    void Start() {
        Debug.Log("导出");
        Export();
    }

    void handleCamera() {
        
    }

    void handleSampler() {

    }

    void handleFilter() {

    }

    void handleAccelerator() {

    }

    void handleFilm() {

    }

    void handleIntegrator() {

    }

    void handleLights() {

    }

    void handlePrimitives() {

    }

    void Export() {
        Camera camera = GameObject.FindObjectOfType<Camera>() as Camera;
        
        MeshFilter[] primitives = GameObject.FindObjectsOfType<MeshFilter>() as MeshFilter[];
        var shape = primitives[0];
        _primitives.Add(shape);

        

        Material material = shape.GetComponent<Renderer>().sharedMaterial;

        Light[] lights = GameObject.FindObjectsOfType<Light>() as Light[];
    }
}

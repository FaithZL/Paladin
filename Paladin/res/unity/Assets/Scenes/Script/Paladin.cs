using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
using LitJson;
using UnityEngine.Assertions;
using System.IO;

public class Paladin : MonoBehaviour {

    public int threadNum = 0;

    [Header("--------filter param--------")]
    public Filter filterName;
    public Vector2 filterRadius = new Vector2(1, 1);
    [Header("Gaussian")]
    public float alpha = 2;
    [Header("Mitchell")]
    public float B = 1.0f / 3.0f;
    public float C = 1.0f / 3.0f;
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

    [Header("Halton,Random")]
    [Header("--------sampler param--------")]

    public Sampler samplerName;
    public int spp = 2;
    [Header("Stratified")]
    public int xSpp = 1;
    public int ySpp = 1;
    public int dimensions = 6;
    public bool jitter = true;

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

    private JsonData _output = new JsonData();


    void Start() {
        Debug.Log("导出");
        exec();
        export();
        Debug.Log("导出完毕");
    }

    void exec() {
        handleCamera();
        handleFilm();
        handleFilter();
        handleSampler();
        handlePrimitives();
        handleIntegrator();
        handleLights();
        handleAccelerator();
        handleThreadNum();
    }

    void handleCamera() {
        
    }

    void handleThreadNum() {
        _output["threadNum"] = threadNum;
    }

    void handleSampler() {
        var samplerData = new JsonData();

        var param = new JsonData();

        samplerData["param"] = param;
        
        switch (samplerName) {
            case Sampler.halton:
                samplerData["type"] = "halton";
                param["spp"] = spp;
                break;
            case Sampler.random:
                samplerData["type"] = "random";
                param["spp"] = spp;
                break;
            case Sampler.stratified:
                samplerData["type"] = "stratified";
                param["xsamples"] = xSpp;
                param["ysamples"] = ySpp;
                param["dimensions"] = dimensions;
                param["jitter"] = jitter;
                break;
        }
        _output["sampler"] = samplerData;
    }

    void handleFilter() {
        var filterData = new JsonData();
        var param = new JsonData();
        filterData["param"] = param;

        var radius = new JsonData();
        radius.Add((double)filterRadius.x);
        radius.Add((double)filterRadius.y);
        param["radius"] = radius;

        switch (filterName) {
            case Filter.box:
                filterData["type"] = "box";
                break;
            case Filter.triange:
                filterData["type"] = "triangle";
                break;
            case Filter.gaussian:
                filterData["type"] = "gaussian";
                param["alpha"] = alpha;
                break;
            case Filter.mitchell:
                filterData["type"] = "mitchell";
                param["B"] = B;
                param["C"] = C;
                break;
            case Filter.sinc:
                filterData["type"] = "sinc";
                param["tau"] = tau;
                break;
        }
        _output["filter"] = filterData;

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

    void export() {
        string json = _output.ToJson(true);
        var dir = "paladin_output";
        if (!Directory.Exists(dir)) {
            Directory.CreateDirectory(dir);
        }
        string SceneFileName = outputName;
        var sr = File.CreateText("./" + dir + "/" + SceneFileName + ".json");

        sr.WriteLine(json);
        sr.Close();
    }
}

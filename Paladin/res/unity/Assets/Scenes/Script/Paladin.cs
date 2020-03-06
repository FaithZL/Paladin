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
    public Vector2Int resolution = new Vector2Int(500,500);
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

        _camera = GameObject.FindObjectOfType<Camera>() as Camera;

        var param = new JsonData();

        var pos = _camera.transform.localPosition;
        var target = _camera.transform.forward + pos;
        var up = _camera.transform.up;

        var lookAt = new JsonData();
        lookAt.Add(fromVec3(pos));
        lookAt.Add(fromVec3(target));
        lookAt.Add(fromVec3(up));

        param["lookAt"] = lookAt;

        param["shutterOpen"] = shutterOpen;
        param["shutterClose"] = shutterClose;
        param["fov"] = _camera.fieldOfView;
        param["lensRadius"] = lensRadius;
        param["focalDistance"] = focalDistance;

        var cameraData = new JsonData();
        cameraData["type"] = "perspective";
        cameraData["param"] = param;

        _output["camera"] = cameraData;

    }

    JsonData fromVec3(Vector3 v) {
        var ret = new JsonData();
        ret.Add((double)v.x);
        ret.Add((double)v.y);
        ret.Add((double)v.z);
        return ret;
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

        filterData["type"] = PaladinEnum.getName(filterName);

        switch (filterName) {
            case Filter.gaussian:
                param["alpha"] = alpha;
                break;
            case Filter.mitchell:
                param["B"] = B;
                param["C"] = C;
                break;
            case Filter.sinc:
                param["tau"] = tau;
                break;
        }
        _output["filter"] = filterData;

    }

    void handleAccelerator() {
        var aclrtData = new JsonData();
        var param = new JsonData();
        aclrtData["param"] = param;
        switch (acceleratorName) {
            case Accelerator.bvh:
                aclrtData["type"] = "bvh";
                param["maxPrimsInNode"] = maxPrimsInNode;
                param["splitMethod"] = PaladinEnum.getName(splitMethod);
                break;
            case Accelerator.kdTree:
                //暂时不支持kdtree
                Assert.IsFalse(false);
                break;
        }

        _output["accelerator"] = aclrtData;
    }

    void handleFilm() {
        var filmData = new JsonData();
        var param = new JsonData();
        filmData["param"] = param;

        var res = new JsonData();
        res.Add((double)resolution.x);
        res.Add((double)resolution.y);
        param["resolution"] = res;

        var cw = new JsonData();
        cw.Add((double)cropWindow.x);
        cw.Add((double)cropWindow.y);
        cw.Add((double)cropWindow.width);
        cw.Add((double)cropWindow.height);
        param["cropWindow"] = cw;

        var fn = outputName + "." + PaladinEnum.getName(fileFormat);
        param["fileName"] = fn;

        param["scale"] = scale;
        param["diagonal"] = diagonal;

        _output["film"] = filmData;
    }

    void handleIntegrator() {
        var integratorData = new JsonData();
        var param = new JsonData();
        integratorData["param"] = param;

        switch (integratorName) {
            case Integrator.path:
                integratorData["type"] = "pt";
                param["maxBounce"] = maxBounce;
                param["rrThreshold"] = rrThreshold;
                param["lightSampleStrategy"] = PaladinEnum.getName(lightSampleStrategy);
                break;
            case Integrator.bdpt:
                integratorData["type"] = "bdpt";
                param["maxBounce"] = maxBounce;
                param["rrThreshold"] = rrThreshold;
                param["lightSampleStrategy"] = PaladinEnum.getName(lightSampleStrategy);
                break;
            case Integrator.volpt:
                integratorData["type"] = "volpt";
                param["maxBounce"] = maxBounce;
                param["rrThreshold"] = rrThreshold;
                param["lightSampleStrategy"] = PaladinEnum.getName(lightSampleStrategy);
                break;
            case Integrator.Geometry:
                integratorData["type"] = "Geometry";
                param["type"] = "normal";
                break;
        }
        _output["integrator"] = integratorData;
    }

    void handleLights() {
        var lightData = new JsonData();
        _output["lights"] = lightData;
        Light[] lights = GameObject.FindObjectsOfType<Light>() as Light[];

        for(int i = 0; i < lights.Length; ++i) {
            Light light = lights[i];
            if(light.type == LightType.Directional
                || light.type == LightType.Point
                || light.type == LightType.Spot) {
                lightData.Add(getLight(light));
            }
        }
    }

    JsonData getLight(Light light) {
        switch (light.type) {
            case LightType.Point:
                return getPointLight(light);
            case LightType.Spot:
                return getSpotLight(light);
            case LightType.Directional:
                return getDistantLight(light);
            default:
                return null;
        }
    }

    JsonData getPointLight(Light light) {
        var ret = new JsonData();
        ret["type"] = "pointLight";

        var param = new JsonData();
        var pos = light.transform.localPosition;
        var tf = new JsonData();
        tf["type"] = "translate";
        tf["param"] = fromVec3(pos);
        param["transform"] = tf;

        var I = new JsonData();
        I["colorType"] = 1;
        var c = light.color;
        I["color"] = fromVec3(new Vector3(c.r, c.g, c.b));
        param["I"] = I;

        param["scale"] = light.intensity;

        ret["param"] = param;

        return ret;
    }

    JsonData getDistantLight(Light light) {
        var ret = new JsonData();

        ret["type"] = "distant";
        var param = new JsonData();
        var fwd = light.transform.forward;
        param["wLight"] = fromVec3(-fwd);

        var L = new JsonData();
        L["colorType"] = 1;
        var c = light.color;
        L["color"] = fromVec3(new Vector3(c.r, c.g, c.b));
        param["L"] = L;

        param["scale"] = light.intensity;

        ret["param"] = param;

        return ret;
    }

    JsonData getSpotLight(Light light) {
        var ret = new JsonData(null);

        // todo

        return ret;
    }

    void handlePrimitives() {
        MeshFilter[] primitives = GameObject.FindObjectsOfType<MeshFilter>() as MeshFilter[];
        var shapeData = new JsonData();
        for(int i = 0; i < primitives.Length; ++i) {
            var prim = primitives[i];
            shapeData.Add(getPrimData(prim));
        }
        _output["shapes"] = shapeData;
    }

    JsonData getPrimData(MeshFilter prim) {
        var ret = new JsonData();

        ret["type"] = "triMesh";
        ret["subType"] = "mesh";

        var param = new JsonData();
        var normals = new JsonData();
        var verts = new JsonData();
        var UVs = new JsonData();
        var material = new JsonData(null);
        var indexes = new JsonData();
        var transformData = new JsonData();

        var mesh = prim.sharedMesh;

        for (int i = 0; i < mesh.normals.Length; ++i) {
            var normal = mesh.normals[i];
            normals.Add((double)normal.x);
            normals.Add((double)normal.y);
            normals.Add((double)normal.z);
        }

        for(int i = 0; i < mesh.vertices.Length; ++i) {
            var vert = mesh.vertices[i];
            verts.Add((double)vert.x);
            verts.Add((double)vert.y);
            verts.Add((double)vert.z);
        }

        for(int i = 0; i < mesh.uv.Length; ++i) {
            var uv = mesh.uv[i];
            UVs.Add((double)uv.x);
            UVs.Add((double)uv.y);
        }

        for (int i = 0; i < mesh.subMeshCount; ++i) {
            var indices = mesh.GetIndices(i);
            for(int j = 0; j < indices.Length; ++j) {
                indexes.Add(indices[j]);
            }
        }

        Matrix4x4 matrix = prim.transform.localToWorldMatrix;

        transformData["type"] = "matrix";
        var matParam = new JsonData();
        for(int i = 0; i < 4; ++i) {
            var row = matrix.GetRow(i);
            matParam.Add((double)row.x);
            matParam.Add((double)row.y);
            matParam.Add((double)row.z);
            matParam.Add((double)row.w);
        }
        transformData["param"] = matParam;

        var mat = prim.GetComponent<Renderer>().sharedMaterial;

        

        ret["param"] = param;
        param["normals"] = normals;
        param["verts"] = verts;
        param["UVs"] = UVs;
        param["material"] = material;
        param["indexes"] = indexes;
        param["transform"] = transformData;

        return ret;
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

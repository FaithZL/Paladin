using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;

public enum Filter
{
    box,
    triange,
    gaussian,
    mitchell,
    sinc
};

public enum Sampler
{
    halton,
    random,
    stratified
}

public enum FileFormat { png,exr }

public class Paladin : MonoBehaviour
{

    [Header("filter param")]
    public Filter filterName;
    [Range(0, 1)]
    public float filterRadius = 1.0f;

    [Header("film param")]
    public Vector2Int res = new Vector2Int(100,100);
    public Rect cropWindow = new Rect(0,0,1,1);
    public string outputName = "paladin";
    public FileFormat fileFormat;
    [Range(0, 1)]
    public float scale = 1.0f;
    [Range(0, 1)]
    public float diagonal = 1.0f;

    [Header("sampler")]
    public Sampler samplerName;


    void Start()
    {

        
    }

    // Update is called once per frame
    void Update()
    {
        
    }

    void Export()
    {
        Camera camera = GameObject.FindObjectOfType<Camera>() as Camera;
        
        MeshFilter[] primitives = GameObject.FindObjectsOfType<MeshFilter>() as MeshFilter[];
        var shape = primitives[0];
        Material material = shape.GetComponent<Renderer>().sharedMaterial;

        var albedo = material.GetColor("Albedo");

        Light[] lights = GameObject.FindObjectsOfType<Light>() as Light[];
    }
}

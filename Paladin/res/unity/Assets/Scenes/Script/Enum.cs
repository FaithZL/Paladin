using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public enum Filter {
    box,
    triange,
    gaussian,
    mitchell,
    sinc
};

public enum Sampler {
    random,
    halton,
    stratified
}

public enum Integrator {
    path,
    bdpt,
    Geometry,
    volpt
}

public enum LightSampleStrategy {
    power,
    uniform
}

public enum Accelerator {
    bvh,
    kdTree
}

public enum AclrtSplitMethod {
    SAH,
    Middle,
    EqualCounts
}

public enum FileFormat {
    png,
    exr
}

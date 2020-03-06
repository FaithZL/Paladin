using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public enum Filter {
    box,
    triangle,
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
    exr,
    jpg
}

class PaladinEnum {

    public string toString() {
        return "PaladinEnum";
    }

    public static string getName(FileFormat type) {
        switch (type) {
            case FileFormat.exr:
                return "exr";
            case FileFormat.png:
                return "png";
            case FileFormat.jpg:
                return "jpg";
            default:
                return "";
        }
    }

    public static string getName(LightSampleStrategy type) {
        switch (type) {
            case LightSampleStrategy.power:
                return "power";
            case LightSampleStrategy.uniform:
                return "uniform";
            default:
                return "";
        }
    }

    public static string getName(Filter type) {
        switch (type) {
            case Filter.box:
                return "box";
            case Filter.triangle:
                return "triangle";
            case Filter.sinc:
                return "sinc";
            case Filter.mitchell:
                return "mitchell";
            case Filter.gaussian:
                return "gaussian";
            default:
                return "";
        }
    }

    public static string getName(AclrtSplitMethod type) {
        switch (type) {
            case AclrtSplitMethod.EqualCounts:
                return "EqualCounts";
            case AclrtSplitMethod.Middle:
                return "Middle";
            case AclrtSplitMethod.SAH:
                return "SAH";
            default:
                return "";
        }
    }
}
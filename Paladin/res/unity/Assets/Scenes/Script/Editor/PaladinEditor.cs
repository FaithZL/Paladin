using System.Collections.Generic;
using UnityEngine;
using UnityEditor;


[CustomEditor(typeof(Paladin))]
public class PaladinEditor : Editor
{

    Paladin paladin;

    void OnEnable()
    {
        //获取当前编辑自定义Inspector的对象
        paladin = (Paladin)target;
        Debug.Log("OnEnable");
    }

    void UpdateSamplerParam()
    {

    }

    void UpdateFilterParam()
    {

    }

    //public override void OnInspectorGUI()
    //{
    //    EditorGUILayout.BeginVertical();

        
    //}
}

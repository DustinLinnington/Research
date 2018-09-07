using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public enum TruthState
{
    TrueFalse = 3,
    True = 2,
    False = 1,
    Invalid = 0
}

public class TruthValidator : MonoBehaviour
{
    // TODO: Use a hashmap instead
    List<string> validityOperators = new List<string>() {"&", "|", "||", "=="};

    // The string represents the name of the truth variable, and the enum represents its possible states:
    // 3 = Can be True or False
    // 2 = Can only be True
    // 1 = Can only be False
    // 0 = Can be neither. This makes the truth statement invalid
    Dictionary<string, TruthState> truthVariables = new Dictionary<string, TruthState>();
    Dictionary<List<string>, string> truthStatements = new Dictionary<List<string>, string>();

    // Use this for initialization
    void Start ()
    {
		
	}
	
	// Update is called once per frame
	void Update ()
    {
		
	}

    public void Validate(Text truthStatement)
    {
        //List<string> truthVariables = new List<string>();

        string unparsedStatement = truthStatement.text;
        string[] splitStatement = unparsedStatement.Split(new string[] { "\n", "\r\n" }, StringSplitOptions.RemoveEmptyEntries);
        foreach (string statementLine in splitStatement)
        {
            bool isValid = true;
            string truthOutput = "Valid if: ";
            string[] elementsInStatement = statementLine.Split(' ');
            for (int i = 0; i < elementsInStatement.Length; i++)
            {
                //if (!validityOperators.Contains(elementsInStatement[i]))
                //{
                //    if (!truthVariables.Contains(elementsInStatement[i]))
                //        truthVariables.Add(elementsInStatement[i]);
                //}
                if (validityOperators.Contains(elementsInStatement[i]))
                {
                    string preOperatorElement = elementsInStatement[i - 1];
                    string postOperatorElement = elementsInStatement[i + 1];

                    PopulateDictionary(preOperatorElement);
                    PopulateDictionary(postOperatorElement);

                    TruthState preOperatorElementTruth = TruthState.True;
                    TruthState postOperatorElementTruth = TruthState.True;
                    if (preOperatorElement[0] == '!')
                    {
                        preOperatorElement.Remove(0, 1);
                        preOperatorElementTruth = TruthState.False;
                    }
                    if (postOperatorElement[0] == '!')
                    {
                        postOperatorElement.Remove(0, 1);
                        postOperatorElementTruth = TruthState.False;
                    }

                    switch (elementsInStatement[i])
                    {
                        case "&":
                            UpdateElement(postOperatorElement, postOperatorElementTruth);
                            UpdateElement(preOperatorElement, preOperatorElementTruth);
                            break;
                        case "|":

                            break;
                    }
                }
            }

            if (isValid)
            {
                Debug.Log("Statement is valid");
                Debug.Log(truthOutput);
            }
        }
    }

    private void PopulateDictionary(string truthVariable)
    {
        if (truthVariables.ContainsKey(truthVariable))
            return;

        truthVariables.Add(truthVariable, TruthState.TrueFalse);
    }

    private void UpdateElement(string truthVariable, TruthState newState)
    {
        if (truthVariables.ContainsKey(truthVariable))
        {
            truthVariables[truthVariable] = newState;
        }
    }
}

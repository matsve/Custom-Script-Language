using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

//using SFML.Window;

namespace System.Data.Script
{
    static class ScriptBinder
    {
        /*private static Script script = new Script();
        private static Graphics graphics;
        private static Skin skin;
        private static Dictionary<string, GuiObject> guiObjects = new Dictionary<string, GuiObject>();

        public static void Bind(Script aScript, Graphics aGraphics, Skin aSkin)
        {
            script = aScript;
            graphics = aGraphics;
            skin = aSkin;
            script.BindNativeFunction("printl", 1, 1, WriteLine);
            script.BindNativeFunction("print", 1, 1, Write);
            script.BindNativeFunction("setPos", 3, 3, SetPos);
            script.BindNativeFunction("setSize", 3, 3, SetSize);

            script.BindExternalType("guiObject", guiObjectInit, guiObjectAssign, guiObjectAsString, guiObjectDelete);
        }
        public static void AddGuiObject(GuiObject Object)
        {
            guiObjects[Object.GetName()] = Object;
        }

        private static string WriteLine(Script.FunctionCall data)
        {
            string text = script.StringParameter(data, 1);
            Console.WriteLine(text);
            return text;
        }
        private static string Write(Script.FunctionCall data)
        {
            string text = script.StringParameter(data, 1);
            Console.Write(text);
            return text;
        }
        private static string SetPos(Script.FunctionCall data)
        {
            string name = script.StringParameter(data, 1);
            float posX = script.FloatParameter(data, 2);
            float posY = script.FloatParameter(data, 3);

            if (guiObjects.ContainsKey(name))
            {
                guiObjects[name].position = new Vector2f(posX, posY);
            }

            return name;
        }
        private static string SetSize(Script.FunctionCall data)
        {
            string name = script.StringParameter(data, 1);
            float posX = script.FloatParameter(data, 2);
            float posY = script.FloatParameter(data, 3);

            if (guiObjects.ContainsKey(name))
            {
                guiObjects[name].size = new Vector2f(posX, posY);
            }

            return name;
        }


        static void guiObjectInit(string name)
        {
            guiObjects[name] = new GuiObject(name, graphics, skin);
        }
        static void guiObjectAssign(string name, string value)
        {
            if (value != "")
                guiObjects[name] = guiObjects[value];
        }
        static string guiObjectAsString(string name)
        {
            return name;
        }
        static void guiObjectDelete(string name)
        {
            guiObjects.Remove(name);
        }*/
    }
}

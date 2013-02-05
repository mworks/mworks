
// For the scripting engine to know what to do with an object, it needs get/set methods
// that work with string names of the variables that need to be set

// This is meant to work just like the design-time interfaces in JavaBeans or VisualBasic


BEGIN_NAMESPACE_MW


class Scriptable{

    void set(char *variable_name, Datum value);
    Datum get(char *variable_name);


}


END_NAMESPACE_MW

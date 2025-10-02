#pragma once

#include <imgui.h>
#include <string>
#include <memory>
#include <vector>

class BaseNode;

class BaseParameter {
public:
    BaseParameter(const std::string& name, const std::string& description = "")
        : m_name(name), m_description(description) {}
    
    virtual ~BaseParameter() = default;
    
    virtual void Render() = 0;
    virtual void SetNode(BaseNode* node) { m_node = node; }
    
    const std::string& GetName() const { return m_name; }
    const std::string& GetDescription() const { return m_description; }
    
protected:
    std::string m_name;
    std::string m_description;
    BaseNode* m_node = nullptr;
};

// Template parameter class for different data types
template<typename T>
class Parameter : public BaseParameter {
public:
    Parameter(const std::string& name, T& value, const std::string& description = "")
        : BaseParameter(name, description), m_value(value) {}
    
    void Render() override {
        // This will be specialized for different types
    }
    
    T& GetValue() { return m_value; }
    const T& GetValue() const { return m_value; }
    
private:
    T& m_value;
};

// Specialized parameter types
class FloatParameter : public BaseParameter {
public:
    FloatParameter(const std::string& name, float& value, float min = 0.0f, float max = 1.0f, 
                   const std::string& format = "%.2f", const std::string& description = "")
        : BaseParameter(name, description), m_value(value), m_min(min), m_max(max), m_format(format) {}
    
    void Render() override;
    
private:
    float& m_value;
    float m_min, m_max;
    std::string m_format;
};

class IntParameter : public BaseParameter {
public:
    IntParameter(const std::string& name, int& value, int min = 0, int max = 100, 
                 const std::string& description = "")
        : BaseParameter(name, description), m_value(value), m_min(min), m_max(max) {}
    
    void Render() override;
    
private:
    int& m_value;
    int m_min, m_max;
};

class ComboParameter : public BaseParameter {
public:
    ComboParameter(const std::string& name, int& value, const std::vector<std::string>& items,
                   const std::string& description = "")
        : BaseParameter(name, description), m_value(value), m_items(items) {}
    
    void Render() override;
    
private:
    int& m_value;
    std::vector<std::string> m_items;
};

class BoolParameter : public BaseParameter {
public:
    BoolParameter(const std::string& name, bool& value, const std::string& description = "")
        : BaseParameter(name, description), m_value(value) {}
    
    void Render() override;
    
private:
    bool& m_value;
};

class Vec2Parameter : public BaseParameter {
public:
    Vec2Parameter(const std::string& name, float* value, float min = 0.0f, float max = 1.0f,
                  const std::string& format = "%.2f", const std::string& description = "")
        : BaseParameter(name, description), m_value(value), m_min(min), m_max(max), m_format(format) {}
    
    void Render() override;
    
private:
    float* m_value;
    float m_min, m_max;
    std::string m_format;
};

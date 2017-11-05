/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"

#include "WavefrontObjParser.h"

#include "Vertex.h"
#include "Attributes.h"
#include "Uniforms.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainContentComponent   : public OpenGLAppComponent, public Button::Listener, public KeyListener
{
public:
	String defaultShaderHeader = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"uniform vec3 iResolution;\n"
		"uniform float iTime;\n"
		"uniform float iTimeDelta;\n"
		"uniform float iFrame;\n"
		"uniform vec4 iMouse;\n"
		"uniform vec4 iDate;\n"
		"uniform float iSampleRate;\n";
	/*
	"uniform float iChannelTime[4];\n"
	"uniform vec3 iChannelResolution[4];\n"
	"uniform samplerxx iChannel[4];\n"
	"uniform sampler1d iChannel1;\n"
	"uniform sampler2d iChannely;\n"
	"uniform sampler3d iChannelz;\n"
	*/
    //==============================================================================

	
    MainContentComponent()
    {
		vertexShader = "#version 330 core\n"
			"layout(location = 0) in vec3 aPos;\n"
			"void main()\n"
			"{\n"
			"gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
			"}\n";

		String defaultShader =
			"void main()\n"
			"{\n"
				"vec2 p = ((gl_FragCoord.xy / iResolution.xy) * 2.0) - 1.0;\n"
				"FragColor = vec4(p.x, p.y, 0.0f, 1.0f);\n"
			"};";


		sourceCode = new CodeDocument();
		tokeniser = new CPlusPlusCodeTokeniser();
		addAndMakeVisible(codeEditorComponent = new CodeEditorComponent(*sourceCode, tokeniser));
		sourceCode->insertText(0, defaultShader);
		codeEditorComponent->setColour(CodeEditorComponent::ColourIds::backgroundColourId, Colours::white.withAlpha(0.4f));

		addAndMakeVisible(generate = new TextButton("Generate"));
		generate->addListener(this);

		addKeyListener(this);

        setSize (800, 600);
    }
    ~MainContentComponent()
    {
        shutdownOpenGL();
		codeEditorComponent = nullptr;
		tokeniser = nullptr;
		sourceCode = nullptr;
    }

    void initialise() override
    {
         createShaders();
    }

    void shutdown() override
    {
        shader = nullptr;
        attributes = nullptr;
        uniforms = nullptr;
    }
    
    Matrix3D<float> getProjectionMatrix() const
    {
        float w = 1.0f / (0.5f + 0.1f);
        float h = w * getLocalBounds().toFloat().getAspectRatio (false);
        return Matrix3D<float>::fromFrustum (-w, w, -h, h, 4.0f, 30.0f);
    }

    Matrix3D<float> getViewMatrix() const
    {
        Matrix3D<float> viewMatrix (Vector3D<float> (0.0f, 0.0f, -10.0f));
        Matrix3D<float> rotationMatrix
            = viewMatrix.rotated (Vector3D<float> (-0.3f* std::sin(getFrameCounter() * 0.02f), 5.0f * std::sin (getFrameCounter() * 0.01f), 0.0f));

        return rotationMatrix * viewMatrix;
    }

	int64 start = Time::currentTimeMillis();
	int64 prev = Time::currentTimeMillis();
    void render() override
    {
        jassert (OpenGLHelpers::isContextActive());
		if (rebuild) {
			createShaders();
			rebuild = false;
		}

		const float desktopScale = (float) openGLContext.getRenderingScale();
		OpenGLHelpers::clear (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
        
		glEnable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		glViewport (0, 0, roundToInt (desktopScale * getWidth()), roundToInt (desktopScale * getHeight()));
		
		shader->use();

		// Setup uniforms
		if (uniforms->time != nullptr) {
			uniforms->time->set((Time::currentTimeMillis() - start) / 1000.0f);
		}

		if (uniforms->timeDelta != nullptr) {
			uniforms->timeDelta->set((prev - start) / 1000.0f);
			prev = Time::currentTimeMillis();
		}

		if (uniforms->date != nullptr) {
			uniforms->date->set(Time::getCurrentTime().getYear(), Time::getCurrentTime().Time::getMonth(), Time::getCurrentTime().Time::getDayOfMonth());
		}

		if (uniforms->frame != nullptr) {
			uniforms->frame->set(getFrameCounter());
		}

		if (uniforms->mouse != nullptr) {
			auto p(Desktop::getInstance().getLastMouseDownPosition().toFloat());
			uniforms->mouse->set(p.getX(), p.getY(), 0.0f, 0.0f);
		}

		if (uniforms->sampleRate != nullptr) {
			uniforms->sampleRate->set(48000.0f);
		}

		if (uniforms->resolution != nullptr) {
			uniforms->resolution->set(getWidth(), getHeight(), 0.0f);
		}

		// Setup quad
		openGLContext.extensions.glBindBuffer(GL_VERTEX_ARRAY, VBO);
		openGLContext.extensions.glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		openGLContext.extensions.glBindVertexArray(VAO);

		// Draw quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		// Reset the element buffers so child Components draw correctly
		openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, 0);
		openGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    
    void paint (Graphics& g) override
    {
        // You can add your component specific drawing code here!
        // This will draw over the top of the openGL background.
    }

    void resized() override
    {
		auto area(getLocalBounds());

		codeEditorComponent->setBounds(getLocalBounds());
		generate->setBounds(area.removeFromBottom(20).removeFromRight(100));
    }

	void createShaders()
    {
		base = defaultShaderHeader.toStdString() + sourceCode->getAllContent().toStdString();
		fragmentShader = base.c_str();

        ScopedPointer<OpenGLShaderProgram> newShader (new OpenGLShaderProgram (openGLContext));
        String statusText;

		if (newShader->addVertexShader(vertexShader) 
			&& newShader->addFragmentShader(fragmentShader)
			&& newShader->link())
		{
			attributes = nullptr;
			uniforms = nullptr;

			shader = newShader;
			shader->use();

			attributes = new Attributes (openGLContext, *shader);
			uniforms   = new Uniforms (openGLContext, *shader);
						

			openGLContext.extensions.glGenVertexArrays(1, &VAO);
			openGLContext.extensions.glGenBuffers(1, &VBO);

			// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
			openGLContext.extensions.glBindVertexArray(VAO);

			openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, VBO);
			openGLContext.extensions.glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

			openGLContext.extensions.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			openGLContext.extensions.glEnableVertexAttribArray(0);

			// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
			openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, 0);

			// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
			// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
			openGLContext.extensions.glBindVertexArray(0);

			statusText = "GLSL: v" + String (OpenGLShaderProgram::getLanguageVersion(), 2);
		}
		else
		{
			statusText = newShader->getLastError();
		}
    }
private:
	bool keyPressed(const KeyPress& key,
		Component* originatingComponent) override {
		if (key == genKey) {
			generate->triggerClick();
			return false;
		}

		if (key == hideKey) {
			codeEditorComponent->setVisible(!codeEditorComponent->isVisible());
			return false;
		}

		return true;
	}

	void buttonClicked(Button* btn) override {
		rebuild = true;
	}

    //==============================================================================
	unsigned int VBO, VAO;
	float vertices[18] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f,  1.0f, 0.0f,
		1.0f,  1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		-1.0f,  -1.0f, 0.0f
	};

    // private member variables
    const char* vertexShader;
    const char* fragmentShader;

    ScopedPointer<OpenGLShaderProgram> shader;
    ScopedPointer<Attributes> attributes;
    ScopedPointer<Uniforms> uniforms;

	ScopedPointer<CodeTokeniser> tokeniser;
	ScopedPointer<CodeDocument> sourceCode;
	ScopedPointer<CodeEditorComponent> codeEditorComponent;

    String newVertexShader, newFragmentShader;

	ScopedPointer<TextButton> generate;

	bool rebuild = false;
	std::string base;

	KeyPress genKey{ 'g', ModifierKeys::ctrlModifier, juce_wchar('g') };
	KeyPress hideKey{ 'h', ModifierKeys::ctrlModifier, juce_wchar('h') };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent()    { return new MainContentComponent(); }

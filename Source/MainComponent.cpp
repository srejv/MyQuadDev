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

static Image resizeImageToPowerOfTwo(Image image)
{
	if (!(isPowerOfTwo(image.getWidth()) && isPowerOfTwo(image.getHeight())))
		return image.rescaled(jmin(1024, nextPowerOfTwo(image.getWidth())),
			jmin(1024, nextPowerOfTwo(image.getHeight())));

	return image;
}

// These classes are used to load textures from the various sources that the demo uses..
struct DemoTexture
{
	virtual ~DemoTexture() {}
	virtual bool applyTo(OpenGLTexture&) = 0;

	String name;
};

struct DynamicTexture : public DemoTexture
{
	DynamicTexture() { name = "Dynamically-generated texture"; }

	Image image;
	//BouncingNumber x, y;

	bool applyTo(OpenGLTexture& texture) override
	{
		const int size = 128;

		if (!image.isValid())
			image = Image(Image::ARGB, size, size, true);

		{
			Graphics g(image);
			g.fillAll(Colours::lightcyan);

			g.setColour(Colours::darkred);
			g.drawRect(0, 0, size, size, 2);

			g.setColour(Colours::green);
			//g.fillEllipse(x.getValue() * size * 0.9f, y.getValue() * size * 0.9f, size * 0.1f, size * 0.1f);

			g.setColour(Colours::black);
			g.setFont(40);
			g.drawFittedText(String(Time::getCurrentTime().getMilliseconds()), image.getBounds(), Justification::centred, 1);
		}

		texture.loadImage(image);
		return true;
	}
};

struct BuiltInTexture : public DemoTexture
{
	BuiltInTexture(const char* nm, const void* imageData, size_t imageSize)
		: image(resizeImageToPowerOfTwo(ImageFileFormat::loadFrom(imageData, imageSize)))
	{
		name = nm;
	}

	Image image;

	bool applyTo(OpenGLTexture& texture) override
	{
		texture.loadImage(image);
		return false;
	}
};

struct TextureFromFile : public DemoTexture
{
	TextureFromFile(const File& file)
	{
		name = file.getFileName();
		image = resizeImageToPowerOfTwo(ImageFileFormat::loadFrom(file));
	}

	Image image;

	bool applyTo(OpenGLTexture& texture) override
	{
		texture.loadImage(image);
		return false;
	}
};

/*
struct TextureFromAudioBuffer : public DemoTexture {
	TextureFromFile(const AudioSampleBuffer&  buffer)
	{
		name = "Audio";
		//image = resizeImageToPowerOfTwo(ImageFileFormat::loadFrom(file));
	}

	Image image;

	bool applyTo(OpenGLTexture& texture) override
	{
		texture.loadImage(image);
		return false;
	}
};
*/

class Renderer : public OpenGLAppComponent {
public:
	//==============================================================================
	Renderer() : OpenGLAppComponent()
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

		setTexture(new BuiltInTexture("Kek", BinaryData::nolie_png, BinaryData::nolie_pngSize));

		base = defaultShaderHeader.toStdString() + defaultShader.toStdString();
	}
	~Renderer()
	{
	}

	void paint(Graphics& g) override{}
	void resized() override {}

	void initialise() override
	{
		//frameBuffer.initialise(openGLContext, getWidth(), getHeight());
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
		float h = w * getLocalBounds().toFloat().getAspectRatio(false);
		return Matrix3D<float>::fromFrustum(-w, w, -h, h, 4.0f, 30.0f);
	}

	Matrix3D<float> getViewMatrix() const
	{
		Matrix3D<float> viewMatrix(Vector3D<float>(0.0f, 0.0f, -10.0f));
		Matrix3D<float> rotationMatrix
			= viewMatrix.rotated(Vector3D<float>(-0.3f* std::sin(getFrameCounter() * 0.02f), 5.0f * std::sin(getFrameCounter() * 0.01f), 0.0f));

		return rotationMatrix * viewMatrix;
	}
	
	void renderFrameBuffer() {
		// ??? WHAT INPUTS? 
		// BUFFER A RENDERSHADERTHINGY
		// frameBuffer.bind();

	}

	void renderMain() {
		const float desktopScale = (float)openGLContext.getRenderingScale();

		if (textureToUse != nullptr)
			if (!textureToUse->applyTo(texture))
				textureToUse = nullptr;

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		openGLContext.extensions.glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);




		glViewport(0, 0, roundToInt(desktopScale * getWidth()), roundToInt(desktopScale * getHeight()));

		texture.bind();

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

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
			uniforms->date->set(static_cast<GLfloat>(Time::getCurrentTime().getYear()),
				static_cast<GLfloat>(Time::getCurrentTime().Time::getMonth()),
				static_cast<GLfloat>(Time::getCurrentTime().Time::getDayOfMonth()));
		}

		if (uniforms->frame != nullptr) {
			uniforms->frame->set(getFrameCounter());
		}

		if (uniforms->channel0 != nullptr)
			uniforms->channel0->set((GLint)0);

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

		//frameBuffer.makeCurrentAndClear();


		// Draw quad
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Reset the element buffers so child Components draw correctly
		openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, 0);
		openGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void render() override
	{
		jassert(OpenGLHelpers::isContextActive());
		if (rebuild) {
			createShaders();
			rebuild = false;
		}

		OpenGLHelpers::clear(Colour(30,30,30));

		renderMain();
	}

	void setBase(std::string newCode) {
		base = defaultShaderHeader.toStdString() + newCode;
		rebuild = true;
	}

	void createShaders()
	{
		fragmentShader = base.c_str();
		ScopedPointer<OpenGLShaderProgram> newShader(new OpenGLShaderProgram(openGLContext));
		String statusText;

		if (newShader->addVertexShader(vertexShader)
			&& newShader->addFragmentShader(fragmentShader)
			&& newShader->link())
		{
			attributes = nullptr;
			uniforms = nullptr;

			shader = newShader;
			shader->use();

			attributes = new Attributes(openGLContext, *shader);
			uniforms = new Uniforms(openGLContext, *shader);


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

			statusText = "GLSL: v" + String(OpenGLShaderProgram::getLanguageVersion(), 2);
		}
		else
		{
			statusText = newShader->getLastError();
		}
	}

	void setTexture(DemoTexture* t)
	{
		lastTexture = textureToUse = t;
	}

private:
	Rectangle<int> mBounds{ 0, 0, 800, 600 };
	//==============================================================================
	String defaultShaderHeader = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"uniform vec3 iResolution;\n"
		"uniform float iTime;\n"
		"uniform float iTimeDelta;\n"
		"uniform float iFrame;\n"
		"uniform vec4 iMouse;\n"
		"uniform vec4 iDate;\n"
		"uniform float iSampleRate;\n"
		"uniform sampler2D iChannel0;\n"
		"uniform sampler2D iFeedback;\n";
	/*
	"uniform float iChannelTime[4];\n"
	"uniform vec3 iChannelResolution[4];\n"
	"uniform samplerxx iChannel[4];\n"
	"uniform sampler1d iChannel1;\n"
	"uniform sampler2d iChannely;\n"
	"uniform sampler3d iChannelz;\n"
	*/

	int64 start = Time::currentTimeMillis();
	int64 prev = Time::currentTimeMillis();

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

	OpenGLTexture texture;
	DemoTexture* textureToUse, *lastTexture;

	//OpenGLFrameBuffer frameBuffer;

	bool rebuild = false;
	std::string base;
};

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainEditorComponent : public Component, public Button::Listener, public KeyListener {
public:
	MainEditorComponent() {
		String defaultShader =
			"void main()\n"
			"{\n"
			"\tvec2 p = ((gl_FragCoord.xy / iResolution.xy) * 2.0) - 1.0;\n"
			"\tFragColor = vec4(p.x, p.y, 0.0f, 1.0f);\n"
			"};";

		sourceCode = new CodeDocument();
		tokeniser = new CPlusPlusCodeTokeniser();
		addAndMakeVisible(codeEditorComponent = new CodeEditorComponent(*sourceCode, tokeniser));
		sourceCode->insertText(0, defaultShader);

		addAndMakeVisible(generate = new TextButton("Generate"));
		generate->addListener(this);

		addAndMakeVisible(savebtn = new TextButton("Save"));
		savebtn->addListener(this);

		addAndMakeVisible(openbtn = new TextButton("Open"));
		openbtn->addListener(this);

		addKeyListener(this);
	}
	~MainEditorComponent() {
		renderer = nullptr;
		codeEditorComponent = nullptr;
		tokeniser = nullptr;
		sourceCode = nullptr;
	}

	void paint(Graphics& g) override {
		g.fillAll(Colour(240, 240, 240));
	}
	void resized() override {
		auto area(getLocalBounds());

		codeEditorComponent->setBounds(getLocalBounds());
		generate->setBounds(area.removeFromBottom(20).removeFromRight(100).reduced(4, 0));
		savebtn->setBounds(area.removeFromBottom(20).removeFromRight(100).reduced(4,0));
		openbtn->setBounds(area.removeFromBottom(20).removeFromRight(100).reduced(4, 0));
	}
	
	void setRenderer(Renderer* therenderer) {
		renderer = therenderer;
	}

	String getFragmentShaderCode() const {
		return sourceCode->getAllContent();
	}

	bool keyPressed(const KeyPress& key, Component* originatingComponent) override {
		ignoreUnused(originatingComponent);
		if (key == genKey) {
			generate->triggerClick();
			return false;
		}

		if (key == hideKey) {
			codeEditorComponent->setVisible(!codeEditorComponent->isVisible());
			return false;
		}

		if (key == saveKey) {
			// Save
			saveFile();
		}
		if (key == openKey) {
			// Open
			openFile();
		}

		return true;
	}

	void buttonClicked(Button* btn) override {
		if (generate == btn) {
			if (renderer) {
				renderer->setBase(sourceCode->getAllContent().toStdString());
			}
		}
		if(btn == savebtn) {
			saveFile();
		}
		if(btn == openbtn) {
			openFile();
		}
	}

	void openFile() {
		WildcardFileFilter wildcardFilter("*.glsl", String(), "Glsl files");
		FileBrowserComponent browser(FileBrowserComponent::canSelectFiles | FileBrowserComponent::openMode,
			File(),
			&wildcardFilter,
			nullptr);
		FileChooserDialogBox dialogBox("Open some kind of file",
			"Please choose some kind of file that you want to open...",
			browser,
			false,
			Colours::lightgrey);
		if (dialogBox.show())
		{
			File selectedFile = browser.getSelectedFile(0);
			ScopedPointer<FileInputStream> inputStream = selectedFile.createInputStream();
			sourceCode->replaceAllContent(inputStream->readString());
		}
	}

	void saveFile() {
		WildcardFileFilter wildcardFilter("*.glsl", String(), "Glsl files");
		FileBrowserComponent browser(FileBrowserComponent::canSelectFiles | FileBrowserComponent::saveMode,
			File(),
			&wildcardFilter,
			nullptr);
		FileChooserDialogBox dialogBox("Save some kind of file",
			"Please choose some kind of file that you want to open...",
			browser,
			false,
			Colours::lightgrey);
		if (dialogBox.show())
		{
			File selectedFile = browser.getSelectedFile(0);
			auto result = selectedFile.create();
			if (result.wasOk()) {
				selectedFile.appendText(sourceCode->getAllContent());
			}
		}
	}

	Renderer* renderer = nullptr;

	ScopedPointer<CodeTokeniser> tokeniser;
	ScopedPointer<CodeDocument> sourceCode;
	ScopedPointer<CodeEditorComponent> codeEditorComponent;

	ScopedPointer<TextButton> generate, savebtn, openbtn;
	
	KeyPress genKey{ 'g', ModifierKeys::commandModifier, juce_wchar('g') };
	KeyPress hideKey{ 'h', ModifierKeys::commandModifier, juce_wchar('h') };
	KeyPress saveKey{ 's', ModifierKeys::commandModifier, juce_wchar('s') };
	KeyPress openKey{ 'o', ModifierKeys::commandModifier, juce_wchar('o') };
};


class MainContentComponent   : public AudioAppComponent
{
public:
    //==============================================================================
    MainContentComponent() {
		editor = new MainEditorComponent();
		editorWindow = new DocumentWindow("Editor", Colours::white, DocumentWindow::TitleBarButtons::allButtons);
		editorWindow->setUsingNativeTitleBar(true);
		editorWindow->setContentNonOwned(editor, false);
		editorWindow->centreWithSize(800, 600);
		editorWindow->setResizable(true, true);
		editorWindow->setVisible(true);

		addAndMakeVisible(renderer = new Renderer());
		editor->setRenderer(renderer);

		setAudioChannels(2, 2);

		setSize (800, 600);
    }

    ~MainContentComponent() {
		editorWindow = nullptr;
		editor = nullptr;
		renderer = nullptr;
	}

    void paint (Graphics& g) override {
		ignoreUnused(g);
	}

    void resized() override {
		auto area(getLocalBounds());
		renderer->setBounds(area);
    }

	void prepareToPlay(int samplesPerBlockExpected,
		double sampleRate) override 
	{
	};

	void releaseResources() override {};

	void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override 
	{
	};

private:
    //==============================================================================
	ScopedPointer<DocumentWindow> editorWindow;
	ScopedPointer<MainEditorComponent> editor;
	ScopedPointer<Renderer> renderer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent()    { return new MainContentComponent(); }

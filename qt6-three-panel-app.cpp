#include <QApplication>
#include <QMainWindow>
#include <QSplitter>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QFormLayout>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QComboBox>
#include <QStatusBar>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QFontMetrics>
#include <QProcess>
#include <QSettings>
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTextCharFormat>
#include <QFileDialog>
#include <QMessageBox>
#include <QTabWidget>
#include <QIcon>
#include <QScrollBar>
#include <QFontDialog>
#include <QDockWidget>
#include <QListWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QToolButton>
#include <QStyle>
#include <QStyleFactory>
#include <QApplication>
#include <QClipboard>
#include <QDateTime>
#include <QPalette>
#include <QColor>
#include <QShortcut>

class CodeHighlighter : public QSyntaxHighlighter {
public:
    CodeHighlighter(QTextDocument *parent = nullptr) : QSyntaxHighlighter(parent) {
        HighlightingRule rule;

        // Keywords
        keywordFormat.setForeground(QColor("#569CD6"));
        keywordFormat.setFontWeight(QFont::Bold);
        QStringList keywordPatterns = {
            "\\bclass\\b", "\\bconst\\b", "\\benum\\b", "\\bexplicit\\b",
            "\\bfriend\\b", "\\binline\\b", "\\bnamespace\\b", "\\boperator\\b",
            "\\bprivate\\b", "\\bprotected\\b", "\\bpublic\\b", "\\bsignals\\b",
            "\\bslots\\b", "\\bstatic\\b", "\\btemplate\\b", "\\btypedef\\b",
            "\\btypename\\b", "\\bunion\\b", "\\bvirtual\\b", "\\bvolatile\\b",
            "\\bbreak\\b", "\\bcase\\b", "\\bcatch\\b", "\\bcontinue\\b",
            "\\bdefault\\b", "\\bdelete\\b", "\\bdo\\b", "\\belse\\b",
            "\\bfor\\b", "\\bif\\b", "\\bnew\\b", "\\breturn\\b",
            "\\bswitch\\b", "\\bthrow\\b", "\\btry\\b", "\\bwhile\\b",
            "\\bauto\\b", "\\bbool\\b", "\\bchar\\b", "\\bdouble\\b",
            "\\bfloat\\b", "\\bint\\b", "\\blong\\b", "\\bshort\\b",
            "\\bsigned\\b", "\\bstruct\\b", "\\bunsigned\\b", "\\bvoid\\b",
            "\\binclude\\b", "\\bdefine\\b", "\\bifdef\\b", "\\bifndef\\b",
            "\\bendif\\b", "\\bundef\\b", "\\bpragma\\b"
        };

        for (const QString &pattern : keywordPatterns) {
            rule.pattern = QRegularExpression(pattern);
            rule.format = keywordFormat;
            highlightingRules.append(rule);
        }

        // Class names
        classFormat.setForeground(QColor("#4EC9B0"));
        rule.pattern = QRegularExpression("\\b[A-Za-z0-9_]+::[A-Za-z0-9_]+\\b");
        rule.format = classFormat;
        highlightingRules.append(rule);

        rule.pattern = QRegularExpression("\\bQ[A-Za-z]+\\b");
        rule.format = classFormat;
        highlightingRules.append(rule);

        // Function calls
        functionFormat.setForeground(QColor("#DCDCAA"));
        rule.pattern = QRegularExpression("\\b[A-Za-z0-9_]+(?=\\()");
        rule.format = functionFormat;
        highlightingRules.append(rule);

        // Single line comments
        singleLineCommentFormat.setForeground(QColor("#6A9955"));
        rule.pattern = QRegularExpression("//[^\n]*");
        rule.format = singleLineCommentFormat;
        highlightingRules.append(rule);

        // Quotations
        quotationFormat.setForeground(QColor("#CE9178"));
        rule.pattern = QRegularExpression("\".*\"");
        rule.format = quotationFormat;
        highlightingRules.append(rule);

        // Numbers
        numberFormat.setForeground(QColor("#B5CEA8"));
        rule.pattern = QRegularExpression("\\b[0-9]+\\b");
        rule.format = numberFormat;
        highlightingRules.append(rule);

        // Preprocessor
        preprocessorFormat.setForeground(QColor("#BD63C5"));
        rule.pattern = QRegularExpression("#[^\n]*");
        rule.format = preprocessorFormat;
        highlightingRules.append(rule);
    }

protected:
    void highlightBlock(const QString &text) override {
        for (const HighlightingRule &rule : highlightingRules) {
            QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
            while (matchIterator.hasNext()) {
                QRegularExpressionMatch match = matchIterator.next();
                setFormat(match.capturedStart(), match.capturedLength(), rule.format);
            }
        }

        // Multi-line comments
        setCurrentBlockState(0);
        
        int startIndex = 0;
        if (previousBlockState() != 1)
            startIndex = text.indexOf("/*");
        
        while (startIndex >= 0) {
            QRegularExpressionMatch match = QRegularExpression("\\*/").match(text, startIndex);
            int endIndex = match.capturedStart();
            int commentLength;
            
            if (endIndex == -1) {
                setCurrentBlockState(1);
                commentLength = text.length() - startIndex;
            } else {
                commentLength = endIndex - startIndex + match.capturedLength();
            }
            
            setFormat(startIndex, commentLength, singleLineCommentFormat);
            startIndex = text.indexOf("/*", startIndex + commentLength);
        }
    }

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat keywordFormat;
    QTextCharFormat classFormat;
    QTextCharFormat functionFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat numberFormat;
    QTextCharFormat preprocessorFormat;
};

// Enhanced code editor with line numbers and syntax highlighting
class CodeEditor : public QPlainTextEdit {
    Q_OBJECT
public:
    CodeEditor(QWidget *parent = nullptr) : QPlainTextEdit(parent) {
        lineNumberArea = new LineNumberArea(this);
        
        connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
        connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateLineNumberArea);
        connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);
        
        updateLineNumberAreaWidth(0);
        highlightCurrentLine();

        // Set monospace font
        QFont font("Cascadia Code", 10);
        if (!QFontInfo(font).fixedPitch()) {
            font.setFamily("Consolas");
            if (!QFontInfo(font).fixedPitch()) {
                font.setFamily("Courier New");
            }
        }
        font.setFixedPitch(true);
        this->setFont(font);

        // Set tab width
        QFontMetrics metrics(font);
        this->setTabStopDistance(4 * metrics.horizontalAdvance(' '));

        // Setup syntax highlighter
        highlighter = new CodeHighlighter(this->document());
        
        // Set dark theme colors
        QPalette p = palette();
        p.setColor(QPalette::Base, QColor("#1E1E1E"));
        p.setColor(QPalette::Text, QColor("#D4D4D4"));
        setPalette(p);
        
        // Line wrapping
        setLineWrapMode(QPlainTextEdit::NoWrap);
        
        // Enable auto-indentation
        connect(this, &CodeEditor::textChanged, this, &CodeEditor::handleTextChanged);
        
        // Set bracket matching
        bracketMatchingEnabled = true;
        bracketPos = -1;
        bracketLength = 0;
    }

    int lineNumberAreaWidth() {
        int digits = 1;
        int max = qMax(1, blockCount());
        while (max >= 10) {
            max /= 10;
            ++digits;
        }

        int space = 20 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
        return space;
    }
    
    void toggleWordWrap() {
        if (lineWrapMode() == QPlainTextEdit::NoWrap) {
            setLineWrapMode(QPlainTextEdit::WidgetWidth);
        } else {
            setLineWrapMode(QPlainTextEdit::NoWrap);
        }
    }
    
    void changeEditorFont() {
        bool ok;
        QFont font = QFontDialog::getFont(&ok, this->font(), this);
        if (ok) {
            this->setFont(font);
            // Update tab stop distance for the new font
            QFontMetrics metrics(font);
            this->setTabStopDistance(4 * metrics.horizontalAdvance(' '));
        }
    }
    
    void toggleBracketMatching() {
        bracketMatchingEnabled = !bracketMatchingEnabled;
        highlightCurrentLine(); // Update to show/hide current bracket matching
    }
    
    bool save(const QString &fileName) {
        QFile file(fileName);
        if (file.open(QFile::WriteOnly | QFile::Text)) {
            QTextStream out(&file);
            out << toPlainText();
            file.close();
            document()->setModified(false);
            return true;
        }
        return false;
    }
    
    bool load(const QString &fileName) {
        QFile file(fileName);
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            setPlainText(file.readAll());
            file.close();
            document()->setModified(false);
            return true;
        }
        return false;
    }

protected:
    void resizeEvent(QResizeEvent *event) override {
        QPlainTextEdit::resizeEvent(event);
        
        QRect cr = contentsRect();
        lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
    }
    
    void keyPressEvent(QKeyEvent *event) override {
        if (event->key() == Qt::Key_Tab) {
            // Insert spaces instead of tab character
            QTextCursor cursor = textCursor();
            cursor.insertText("    ");
            event->accept();
        } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            // Auto-indentation
            QTextCursor cursor = textCursor();
            QString currentLine = cursor.block().text();
            
            // Find leading whitespace
            int indentSize = 0;
            while (indentSize < currentLine.length() && currentLine.at(indentSize).isSpace()) {
                indentSize++;
            }
            
            // Default indent
            QString indent = currentLine.left(indentSize);
            
            // Check for conditions that would increase indentation (like ending with '{')
            if (currentLine.trimmed().endsWith("{") || 
                currentLine.trimmed().endsWith(":")) {
                indent += "    ";
            }
            
            QPlainTextEdit::keyPressEvent(event);
            cursor = textCursor();
            cursor.insertText(indent);
        } else {
            QPlainTextEdit::keyPressEvent(event);
        }
        
        // Update bracket matching
        if (bracketMatchingEnabled) {
            matchBrackets();
        }
    }

private slots:
    void updateLineNumberAreaWidth(int newBlockCount) {
        setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
    }

    void updateLineNumberArea(const QRect &rect, int dy) {
        if (dy)
            lineNumberArea->scroll(0, dy);
        else
            lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

        if (rect.contains(viewport()->rect()))
            updateLineNumberAreaWidth(0);
    }
    
    void highlightCurrentLine() {
        QList<QTextEdit::ExtraSelection> extraSelections;

        // Highlight current line
        if (!isReadOnly()) {
            QTextEdit::ExtraSelection selection;
            QColor lineColor = QColor("#2D2D30").lighter(120);
            selection.format.setBackground(lineColor);
            selection.format.setProperty(QTextFormat::FullWidthSelection, true);
            selection.cursor = textCursor();
            selection.cursor.clearSelection();
            extraSelections.append(selection);
        }
        
        // Add bracket matching highlighting if enabled
        if (bracketMatchingEnabled && bracketPos != -1) {
            QTextEdit::ExtraSelection selection;
            selection.format.setBackground(QColor("#664B0082"));
            selection.format.setForeground(Qt::white);
            selection.cursor = textCursor();
            selection.cursor.setPosition(bracketPos);
            selection.cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, bracketLength);
            extraSelections.append(selection);
        }
        
        setExtraSelections(extraSelections);
    }
    
    void handleTextChanged() {
        // Perform any needed actions when text changes
        if (bracketMatchingEnabled) {
            matchBrackets();
        }
    }
    
    void matchBrackets() {
        bracketPos = -1;
        bracketLength = 0;
        
        QTextCursor cursor = textCursor();
        int position = cursor.position();
        
        if (position == 0) {
            highlightCurrentLine();
            return;
        }
        
        // Check character before cursor
        cursor.setPosition(position - 1);
        QString text = toPlainText();
        QChar ch = text.at(position - 1);
        
        if (ch == '(' || ch == '{' || ch == '[') {
            // Find matching closing bracket
            QChar openBracket = ch;
            QChar closeBracket;
            if (ch == '(') closeBracket = ')';
            else if (ch == '{') closeBracket = '}';
            else closeBracket = ']';
            
            int count = 1;
            for (int i = position; i < text.length(); i++) {
                if (text.at(i) == openBracket) count++;
                else if (text.at(i) == closeBracket) count--;
                
                if (count == 0) {
                    bracketPos = i;
                    bracketLength = 1;
                    break;
                }
            }
        } else if (ch == ')' || ch == '}' || ch == ']') {
            // Find matching opening bracket
            QChar closeBracket = ch;
            QChar openBracket;
            if (ch == ')') openBracket = '(';
            else if (ch == '}') openBracket = '{';
            else openBracket = '[';
            
            int count = 1;
            for (int i = position - 2; i >= 0; i--) {
                if (text.at(i) == closeBracket) count++;
                else if (text.at(i) == openBracket) count--;
                
                if (count == 0) {
                    bracketPos = i;
                    bracketLength = 1;
                    break;
                }
            }
        }
        
        highlightCurrentLine();
    }

    void lineNumberAreaPaintEvent(QPaintEvent *event) {
        QPainter painter(lineNumberArea);
        painter.fillRect(event->rect(), QColor("#1E1E1E"));

        QTextBlock block = firstVisibleBlock();
        int blockNumber = block.blockNumber();
        int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
        int bottom = top + qRound(blockBoundingRect(block).height());

        while (block.isValid() && top <= event->rect().bottom()) {
            if (block.isVisible() && bottom >= event->rect().top()) {
                QString number = QString::number(blockNumber + 1);
                painter.setPen(QColor("#858585"));
                painter.drawText(0, top, lineNumberArea->width() - 5, fontMetrics().height(),
                                 Qt::AlignRight, number);
            }

            block = block.next();
            top = bottom;
            bottom = top + qRound(blockBoundingRect(block).height());
            ++blockNumber;
        }
    }

private:
    class LineNumberArea : public QWidget {
    public:
        LineNumberArea(CodeEditor *editor) : QWidget(editor), codeEditor(editor) {}

        QSize sizeHint() const override {
            return QSize(codeEditor->lineNumberAreaWidth(), 0);
        }

    protected:
        void paintEvent(QPaintEvent *event) override {
            codeEditor->lineNumberAreaPaintEvent(event);
        }

    private:
        CodeEditor *codeEditor;
    };

    LineNumberArea *lineNumberArea;
    CodeHighlighter *highlighter;
    bool bracketMatchingEnabled;
    int bracketPos;
    int bracketLength;
};

// File tree widget to display project structure
class ProjectTreeWidget : public QTreeWidget {
    Q_OBJECT
public:
    ProjectTreeWidget(QWidget *parent = nullptr) : QTreeWidget(parent) {
        setHeaderLabel("Project Files");
        setColumnCount(1);
        setSortingEnabled(true);
        
        // Set style
        setStyleSheet("QTreeWidget { background-color: #252526; color: #D4D4D4; border: none; }");
        
        // Add sample project structure
        addSampleProjectStructure();
        
        // Connect signals
        connect(this, &QTreeWidget::itemDoubleClicked, this, &ProjectTreeWidget::onItemDoubleClicked);
    }
    
    void addSampleProjectStructure() {
        // Root item
        QTreeWidgetItem *projectRoot = new QTreeWidgetItem(this);
        projectRoot->setText(0, "Project");
        projectRoot->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
        projectRoot->setExpanded(true);
        
        // Source directory
        QTreeWidgetItem *srcDir = new QTreeWidgetItem(projectRoot);
        srcDir->setText(0, "src");
        srcDir->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
        srcDir->setExpanded(true);
        
        // Source files
        QTreeWidgetItem *mainCpp = new QTreeWidgetItem(srcDir);
        mainCpp->setText(0, "main.cpp");
        mainCpp->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));
        
        QTreeWidgetItem *appCpp = new QTreeWidgetItem(srcDir);
        appCpp->setText(0, "application.cpp");
        appCpp->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));
        
        QTreeWidgetItem *appH = new QTreeWidgetItem(srcDir);
        appH->setText(0, "application.h");
        appH->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));
        
        // Include directory
        QTreeWidgetItem *includeDir = new QTreeWidgetItem(projectRoot);
        includeDir->setText(0, "include");
        includeDir->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
        
        // Build directory
        QTreeWidgetItem *buildDir = new QTreeWidgetItem(projectRoot);
        buildDir->setText(0, "build");
        buildDir->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
        
        // Config files
        QTreeWidgetItem *cmakeFile = new QTreeWidgetItem(projectRoot);
        cmakeFile->setText(0, "CMakeLists.txt");
        cmakeFile->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));
        
        QTreeWidgetItem *readmeFile = new QTreeWidgetItem(projectRoot);
        readmeFile->setText(0, "README.md");
        readmeFile->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));
    }

signals:
    void openFile(const QString &filePath);

private slots:
    void onItemDoubleClicked(QTreeWidgetItem *item, int column) {
        // Ignore directories
        if (!item || item->childCount() > 0) {
            return;
        }
        
        // Get full path - in a real app, you'd build this from the actual file system
        QString filePath = getItemPath(item);
        emit openFile(filePath);
    }
    
private:
    QString getItemPath(QTreeWidgetItem *item) {
        if (!item) {
            return QString();
        }
        
        QString path = item->text(0);
        QTreeWidgetItem *parent = item->parent();
        
        while (parent) {
            path = parent->text(0) + "/" + path;
            parent = parent->parent();
        }
        
        return path;
    }
};

// Enhanced terminal widget with better styling and features
class TerminalWidget : public QWidget {
    Q_OBJECT
public:
    TerminalWidget(QWidget *parent = nullptr) : QWidget(parent) {
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        
        // Command history
        commandHistory = QStringList();
        historyIndex = -1;
        
        // Terminal toolbar
        QHBoxLayout *toolbarLayout = new QHBoxLayout();
        toolbarLayout->setContentsMargins(5, 5, 5, 5);
        
        QLabel *termLabel = new QLabel("Terminal", this);
        termLabel->setStyleSheet("color: #BBBBBB; font-weight: bold;");
        
        clearButton = new QToolButton(this);
        clearButton->setIcon(style()->standardIcon(QStyle::SP_DialogResetButton));
        clearButton->setToolTip("Clear Terminal");
        connect(clearButton, &QToolButton::clicked, this, &TerminalWidget::clearTerminal);
        
        QToolButton *configButton = new QToolButton(this);
        configButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
        configButton->setToolTip("Terminal Settings");
        
        toolbarLayout->addWidget(termLabel);
        toolbarLayout->addStretch();
        toolbarLayout->addWidget(clearButton);
        toolbarLayout->addWidget(configButton);
        
        // Terminal output display
        outputDisplay = new QPlainTextEdit(this);
        outputDisplay->setReadOnly(true);
        QFont font("Cascadia Code", 10);
        if (!QFontInfo(font).fixedPitch()) {
            font.setFamily("Consolas");
            if (!QFontInfo(font).fixedPitch()) {
                font.setFamily("Courier New");
            }
        }
        font.setFixedPitch(true);
        outputDisplay->setFont(font);
        outputDisplay->setStyleSheet("QPlainTextEdit { background-color: #1E1E1E; color: #D4D4D4; border: none; }");
        
        // Command prompt layout
        QHBoxLayout *promptLayout = new QHBoxLayout();
        promptLayout->setContentsMargins(5, 5, 5, 5);
        
        // Prompt label
        QLabel *promptLabel = new QLabel(">", this);
        promptLabel->setStyleSheet("color: #569CD6; font-weight: bold;");
        
        // Command input
        commandInput = new QLineEdit(this);
        commandInput->setStyleSheet("QLineEdit { background-color: #1E1E1E; color: #D4D4D4; border: none; }");
        commandInput->setFont(font);
        
        promptLayout->addWidget(promptLabel);
        promptLayout->addWidget(commandInput);
        
        // Add all components to main layout
        layout->addLayout(toolbarLayout);
        layout->addWidget(outputDisplay);
        layout->addLayout(promptLayout);
        
        // Set up process
        process = new QProcess(this);
        connect(process, &QProcess::readyReadStandardOutput, this, &TerminalWidget::readOutput);
        connect(process, &QProcess::readyReadStandardError, this, &TerminalWidget::readError);
        
        // Connect command input
        connect(commandInput, &QLineEdit::returnPressed, this, &TerminalWidget::executeCommand);
        
        // Set up keyboard shortcuts for history navigation
        QShortcut *upShortcut = new QShortcut(Qt::Key_Up, commandInput);
        connect(upShortcut, &QShortcut::activated, this, &TerminalWidget::navigateHistoryUp);
        
        QShortcut *downShortcut = new QShortcut(Qt::Key_Down, commandInput);
        connect(downShortcut, &QShortcut::activated, this, &TerminalWidget::navigateHistoryDown);
        
        // Welcome message
        outputDisplay->appendHtml("<span style='color: #569CD6;'>Terminal Ready</span>");
        outputDisplay->appendPlainText("Type 'help' for available commands");
        outputDisplay->appendPlainText("-------------------------------------");
    }

private slots:
    void executeCommand() {
        QString command = commandInput->text().trimmed();
        if (command.isEmpty()) {
            return;
        }
        
        // Add to history
        addToHistory(command);
        
        // Clear input
        commandInput->clear();
        
        outputDisplay->appendHtml("<span style='color: #DCDCAA;'>> " + command.toHtmlEscaped() + "</span>");
        
        // Handle built-in commands
        if (command == "clear" || command == "cls") {
            clearTerminal();
            return;
        } else if (command == "help") {
            showHelp();
            return;
        } else if (command.startsWith("echo ")) {
            outputDisplay->appendPlainText(command.mid(5));
            return;
        }
        
        // Execute external command
#ifdef Q_OS_WIN
        process->start("cmd.exe", QStringList() << "/c" << command);
#else
        process->start("/bin/sh", QStringList() << "-c" << command);
#endif
    }
    
    void clearTerminal() {
        outputDisplay->clear();
    }
    
    void showHelp() {
        outputDisplay->appendHtml("<span style='color: #569CD6;'>Available Commands:</span>");
        outputDisplay->appendHtml("<span style='color: #DCDCAA;'>clear/cls</span> - Clear terminal output");
        outputDisplay->appendHtml("<span style='color: #DCDCAA;'>echo [text]</span> - Display text");
        outputDisplay->appendHtml("<span style='color: #DCDCAA;'>help</span> - Show this help message");
        outputDisplay->appendPlainText("Any other command will be executed in the system shell");
    }
    
    void readOutput() {
        QByteArray output = process->readAllStandardOutput();
        outputDisplay->appendPlainText(QString::fromLocal8Bit(output));
    }
    
    void readError() {
        QByteArray error = process->readAllStandardError();
        outputDisplay->appendHtml("<span style='color: #F14C4C;'>" + 
                                  QString::fromLocal8Bit(error).toHtmlEscaped() + "</span>");
    }
    
    void addToHistory(const QString &command) {
        // Don't add duplicates consecutively
        if (!commandHistory.isEmpty() && commandHistory.last() == command) {
            historyIndex = commandHistory.size();
            return;
        }
        
        commandHistory.append(command);
        historyIndex = commandHistory.size();
        
        // Limit history size
        if (commandHistory.size() > 50) {
            commandHistory.removeFirst();
        }
    }
    
    void navigateHistoryUp() {
        if (commandHistory.isEmpty() || historyIndex <= 0) {
            return;
        }
        
        // Store current input if at end of history
        if (historyIndex == commandHistory.size()) {
            currentInput = commandInput->text();
        }
        
        historyIndex--;
        commandInput->setText(commandHistory.at(historyIndex));
    }
    
    void navigateHistoryDown() {
        if (historyIndex >= commandHistory.size()) {
            return;
        }
        
        historyIndex++;
        
        if (historyIndex == commandHistory.size()) {
            commandInput->setText(currentInput);
        } else {
            commandInput->setText(commandHistory.at(historyIndex));
        }
    }

private:
    QPlainTextEdit *outputDisplay;
    QLineEdit *commandInput;
    QToolButton *clearButton;
    QProcess *process;
    QStringList commandHistory;
    int historyIndex;
    QString currentInput;
};

// Gemini API client widget
class GeminiWidget : public QWidget {
    Q_OBJECT
public:
    GeminiWidget(QWidget *parent = nullptr) : QWidget(parent) {
        QVBoxLayout *layout = new QVBoxLayout(this);
        
        // API key configuration
        QGroupBox *configGroup = new QGroupBox("Gemini API Configuration", this);
        QFormLayout *configLayout = new QFormLayout(configGroup);
        
        apiKeyInput = new QLineEdit(this);
        apiKeyInput->setEchoMode(QLineEdit::Password);
        apiKeyInput->setPlaceholderText("Enter your Gemini API key");
        configLayout->addRow("API Key:", apiKeyInput);
        
        modelSelector = new QComboBox(this);
        modelSelector->addItems({"gemini-pro", "gemini-ultra"});
        configLayout->addRow("Model:", modelSelector);
        
        saveConfigButton = new QPushButton("Save Configuration", this);
        connect(saveConfigButton, &QPushButton::clicked, this, &GeminiWidget::saveConfig);
        configLayout->addRow("", saveConfigButton);
        
        // Chat interface
        QGroupBox *chatGroup = new QGroupBox("Gemini Chat", this);
        QVBoxLayout *chatLayout = new QVBoxLayout(chatGroup);
        
        chatOutput = new QTextEdit(this);
        chatOutput->setReadOnly(true);
        chatLayout->addWidget(chatOutput);
        
        chatInput = new QTextEdit(this);
        chatInput->setPlaceholderText("Type your message to Gemini...");
        chatInput->setMaximumHeight(100);
        chatLayout->addWidget(chatInput);
        
        QPushButton *sendButton = new QPushButton("Send", this);
        connect(sendButton, &QPushButton::clicked, this, &GeminiWidget::sendMessage);
        chatLayout->addWidget(sendButton);
        
        // Add all components to main layout
        layout->addWidget(configGroup);
        layout->addWidget(chatGroup);
        
        // Set up network manager for API requests
        //networkManager = new QNetworkAccessManager(this);
        //connect(networkManager, &QNetworkAccessManager::finished, this, &GeminiWidget::handleApiResponse);
        
        // Load saved configuration
        loadConfig();
    }

private slots:
    void saveConfig() {
        QSettings settings("MyDevApp", "GeminiAPI");
        settings.setValue("apiKey", apiKeyInput->text());
        settings.setValue("model", modelSelector->currentText());
        
        chatOutput->append("<i>Configuration saved.</i>");
    }
    
    void loadConfig() {
        QSettings settings("MyDevApp", "GeminiAPI");
        apiKeyInput->setText(settings.value("apiKey").toString());
        
        QString savedModel = settings.value("model").toString();
        if (!savedModel.isEmpty()) {
            int index = modelSelector->findText(savedModel);
            if (index >= 0) {
                modelSelector->setCurrentIndex(index);
            }
        }
    }
    
    void sendMessage() {
        QString message = chatInput->toPlainText().trimmed();
        if (message.isEmpty()) return;
        
        chatOutput->append("<b>You:</b> " + message);
        chatInput->clear();
        
        // Create API request
        QString apiKey = apiKeyInput->text();
        if (apiKey.isEmpty()) {
            chatOutput->append("<span style='color: red;'>Error: API key is required!</span>");
            return;
        }
        
        QString model = modelSelector->currentText();
        QString endpoint = "https://generativelanguage.googleapis.com/v1/models/" + model + ":generateContent?key=" + apiKey;
        
        QNetworkRequest request(QUrl(endpoint));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        
        QJsonObject requestBody;
        QJsonArray contents;
        QJsonObject content;
        content["role"] = "user";
        content["parts"] = QJsonArray{QJsonObject{{"text", message}}};
        contents.append(content);
        requestBody["contents"] = contents;
        
        QJsonDocument doc(requestBody);
        
        chatOutput->append("<i>Waiting for Gemini response...</i>");
        networkManager->post(request, doc.toJson());
    }
    
    void handleApiResponse(QNetworkReply *reply) {
        if (reply->error() != QNetworkReply::NoError) {
            chatOutput->append("<span style='color: red;'>Error: " + reply->errorString() + "</span>");
            reply->deleteLater();
            return;
        }
        
        QByteArray responseData = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            
            if (obj.contains("candidates")) {
                QJsonArray candidates = obj["candidates"].toArray();
                if (!candidates.isEmpty()) {
                    QJsonObject candidate = candidates.first().toObject();
                    QJsonObject content = candidate["content"].toObject();
                    QJsonArray parts = content["parts"].toArray();
                    if (!parts.isEmpty()) {
                        QString text = parts.first().toObject()["text"].toString();
                        chatOutput->append("<b>Gemini:</b> " + text);
                    }
                }
            } else if (obj.contains("error")) {
                QJsonObject error = obj["error"].toObject();
                chatOutput->append("<span style='color: red;'>API Error: " + 
                                  error["message"].toString() + "</span>");
            }
        }
        
        reply->deleteLater();
    }

private:
    QLineEdit *apiKeyInput;
    QComboBox *modelSelector;
    QPushButton *saveConfigButton;
    QTextEdit *chatOutput;
    QTextEdit *chatInput;
    QNetworkAccessManager *networkManager;
};

// Main application window
class MainWindow : public QMainWindow {
public:
    MainWindow() : QMainWindow() {
        setWindowTitle("Development Environment");
        resize(1200, 800);
        
        // Create central widget and main splitter
        QWidget *centralWidget = new QWidget(this);
        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
        
        QSplitter *mainSplitter = new QSplitter(Qt::Horizontal, this);
        
        // IDE panel
        QWidget *idePanel = new QWidget(this);
        QVBoxLayout *ideLayout = new QVBoxLayout(idePanel);
        
        QLabel *ideTitle = new QLabel("Code Editor", this);
        ideTitle->setStyleSheet("font-weight: bold; font-size: 14px;");
        
        CodeEditor *codeEditor = new CodeEditor(this);
        codeEditor->setPlaceholderText("Write your code here...");
        
        ideLayout->addWidget(ideTitle);
        ideLayout->addWidget(codeEditor);
        
        // Gemini API panel
        QWidget *geminiPanel = new QWidget(this);
        QVBoxLayout *geminiLayout = new QVBoxLayout(geminiPanel);
        
        QLabel *geminiTitle = new QLabel("Gemini AI", this);
        geminiTitle->setStyleSheet("font-weight: bold; font-size: 14px;");
        
        GeminiWidget *geminiWidget = new GeminiWidget(this);
        
        geminiLayout->addWidget(geminiTitle);
        geminiLayout->addWidget(geminiWidget);
        
        // Terminal panel
        QWidget *terminalPanel = new QWidget(this);
        QVBoxLayout *terminalLayout = new QVBoxLayout(terminalPanel);
        
        QLabel *terminalTitle = new QLabel("Terminal", this);
        terminalTitle->setStyleSheet("font-weight: bold; font-size: 14px;");
        
        TerminalWidget *terminal = new TerminalWidget(this);
        
        terminalLayout->addWidget(terminalTitle);
        terminalLayout->addWidget(terminal);
        
        // Add panels to splitter
        mainSplitter->addWidget(idePanel);
        mainSplitter->addWidget(geminiPanel);
        mainSplitter->addWidget(terminalPanel);
        mainSplitter->setSizes(QList<int>() << 400 << 400 << 400);
        
        mainLayout->addWidget(mainSplitter);
        setCentralWidget(centralWidget);
        
        // Create menu bar
        setupMenus();
        
        // Create status bar
        statusBar()->showMessage("Ready");
    }

private:
    void setupMenus() {
        // File menu
        QMenu *fileMenu = menuBar()->addMenu("&File");
        
        QAction *newAction = fileMenu->addAction("&New");
        newAction->setShortcut(QKeySequence::New);
        
        QAction *openAction = fileMenu->addAction("&Open");
        openAction->setShortcut(QKeySequence::Open);
        
        QAction *saveAction = fileMenu->addAction("&Save");
        saveAction->setShortcut(QKeySequence::Save);
        
        fileMenu->addSeparator();
        
        QAction *exitAction = fileMenu->addAction("E&xit");
        exitAction->setShortcut(QKeySequence::Quit);
        connect(exitAction, &QAction::triggered, this, &MainWindow::close);
        
        // Edit menu
        QMenu *editMenu = menuBar()->addMenu("&Edit");
        
        QAction *undoAction = editMenu->addAction("&Undo");
        undoAction->setShortcut(QKeySequence::Undo);
        
        QAction *redoAction = editMenu->addAction("&Redo");
        redoAction->setShortcut(QKeySequence::Redo);
        
        editMenu->addSeparator();
        
        QAction *cutAction = editMenu->addAction("Cu&t");
        cutAction->setShortcut(QKeySequence::Cut);
        
        QAction *copyAction = editMenu->addAction("&Copy");
        copyAction->setShortcut(QKeySequence::Copy);
        
        QAction *pasteAction = editMenu->addAction("&Paste");
        pasteAction->setShortcut(QKeySequence::Paste);
        
        // View menu
        QMenu *viewMenu = menuBar()->addMenu("&View");
        
        QAction *toggleTerminalAction = viewMenu->addAction("Toggle &Terminal");
        toggleTerminalAction->setCheckable(true);
        toggleTerminalAction->setChecked(true);
        
        QAction *toggleGeminiAction = viewMenu->addAction("Toggle &Gemini Panel");
        toggleGeminiAction->setCheckable(true);
        toggleGeminiAction->setChecked(true);
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Development Environment");
    
    MainWindow mainWindow;
    mainWindow.show();
    
    return app.exec();
}

#include "main.moc"

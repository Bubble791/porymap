#include "multikeyedit.h"

#include <QLineEdit>
#include <QHBoxLayout>
#include <QtEvents>
#include <QMenu>
#include <QAction>


MultiKeyEdit::MultiKeyEdit(QWidget *parent, int fieldCount) :
    QWidget(parent)
{
    setLayout(new QHBoxLayout(this));
    layout()->setContentsMargins(0, 0, 0, 0);
    setFieldCount(fieldCount);
}

bool MultiKeyEdit::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
        auto *watched_kse = qobject_cast<QKeySequenceEdit *>(watched);
        if (!watched_kse)
            return false;

        auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            watched_kse->clearFocus();
            return true;
        } else {
            watched_kse->clear();
        }
    }
    
    if (event->type() == QEvent::ContextMenu) {
        auto *watched_lineEdit = qobject_cast<QLineEdit *>(watched);
        if (!watched_lineEdit)
            return false;

        auto *contextMenuEvent = static_cast<QContextMenuEvent *>(event);
        if (contextMenuPolicy() == Qt::DefaultContextMenu) {
            showDefaultContextMenu(watched_lineEdit, contextMenuEvent->pos());
            return true;
        }
    }

    return false;
}

int MultiKeyEdit::fieldCount() const {
    return this->keySequenceEdits.count();
}

void MultiKeyEdit::setFieldCount(int count) {
    if (count < 1)
        count = 1;

    while (this->keySequenceEdits.count() < count)
        addNewKeySequenceEdit();

    while (this->keySequenceEdits.count() > count)
        delete this->keySequenceEdits.takeLast();

    alignKeySequencesLeft();
}

QList<QKeySequence> MultiKeyEdit::keySequences() const {
    QList<QKeySequence> current_keySequences;
    for (auto *kse : this->keySequenceEdits)
        if (!kse->keySequence().isEmpty())
            current_keySequences.append(kse->keySequence());
    return current_keySequences;
}

bool MultiKeyEdit::removeOne(const QKeySequence &keySequence) {
    for (auto *keySequenceEdit : this->keySequenceEdits) {
        if (keySequenceEdit->keySequence() == keySequence) {
            keySequenceEdit->clear();
            alignKeySequencesLeft();
            setFocusToLastNonEmptyKeySequenceEdit();
            return true;
        }
    }
    return false;
}

bool MultiKeyEdit::contains(const QKeySequence &keySequence) const {
    for (auto current_keySequence : keySequences())
        if (current_keySequence == keySequence)
            return true;
    return false;
}

void MultiKeyEdit::setContextMenuPolicy(Qt::ContextMenuPolicy policy) {
    QWidget::setContextMenuPolicy(policy);
    auto lineEdit_children = findChildren<QLineEdit *>();
    for (auto *lineEdit : lineEdit_children)
        lineEdit->setContextMenuPolicy(policy);
}

bool MultiKeyEdit::isClearButtonEnabled() const {
    return findChild<QLineEdit *>()->isClearButtonEnabled();
}

void MultiKeyEdit::setClearButtonEnabled(bool enable) {
    for (auto *lineEdit : findChildren<QLineEdit *>())
        lineEdit->setClearButtonEnabled(enable);
}

void MultiKeyEdit::clear() {
    for (auto *keySequenceEdit : this->keySequenceEdits)
        keySequenceEdit->clear();
}

void MultiKeyEdit::setKeySequences(const QList<QKeySequence> &keySequences) {
    clear();
    int minCount = qMin(this->keySequenceEdits.count(), keySequences.count());
    for (int i = 0; i < minCount; ++i)
        this->keySequenceEdits[i]->setKeySequence(keySequences.at(i));
}

void MultiKeyEdit::addKeySequence(const QKeySequence &keySequence) {
    this->keySequenceEdits.last()->setKeySequence(keySequence);
    alignKeySequencesLeft();
}

void MultiKeyEdit::addNewKeySequenceEdit() {
    auto *keySequenceEdit = new QKeySequenceEdit(this);
    keySequenceEdit->installEventFilter(this);
    connect(keySequenceEdit, &QKeySequenceEdit::editingFinished, [this, keySequenceEdit] {
        onEditingFinished(keySequenceEdit);
    });
    connect(keySequenceEdit, &QKeySequenceEdit::keySequenceChanged,
            this, &MultiKeyEdit::keySequenceChanged);

    auto *lineEdit = keySequenceEdit->findChild<QLineEdit *>();
    lineEdit->setClearButtonEnabled(true);
    lineEdit->installEventFilter(this);
    connect(lineEdit, &QLineEdit::customContextMenuRequested,
            this, &MultiKeyEdit::customContextMenuRequested);

    // Gross way to connect the line edit's clear button.
    auto actions = lineEdit->findChildren<QAction*>();
    if (!actions.isEmpty()) {
        connect(actions.first(), &QAction::triggered, this, [this, keySequenceEdit]() {
            removeOne(keySequenceEdit->keySequence());
        });
    }

    layout()->addWidget(keySequenceEdit);
    this->keySequenceEdits.append(keySequenceEdit);
}

// Shift all key sequences left if there are any empty QKeySequenceEdit's.
void MultiKeyEdit::alignKeySequencesLeft() {
    blockSignals(true);
    setKeySequences(keySequences());
    blockSignals(false);
}

void MultiKeyEdit::setFocusToLastNonEmptyKeySequenceEdit() {
    for (auto it = this->keySequenceEdits.rbegin(); it != this->keySequenceEdits.rend(); ++it) {
        if (!(*it)->keySequence().isEmpty()) {
            (*it)->setFocus();
            return;
        }
    }
}

void MultiKeyEdit::onEditingFinished(QKeySequenceEdit *sender) {
    if (!sender) return;

    alignKeySequencesLeft();

    // Remove duplicate key sequences, if any
    if (!sender->keySequence().isEmpty()) {
        for (const auto &edit : this->keySequenceEdits) {
            if (edit != sender && edit->keySequence() == sender->keySequence()) {
                removeOne(edit->keySequence());
            }
        }
    }

    emit editingFinished();
}

/* QKeySequenceEdit doesn't send or receive context menu events, but it owns QLineEdit that does.
 * This QLineEdit hijacks those events and so we need to filter/connect to it directly, rather than
 * the QKeySequenceEdit. I wouldn't be surprised if Qt fixed this in the future, in which case any
 * context menu related code in this class might need to change. */
void MultiKeyEdit::showDefaultContextMenu(QLineEdit *lineEdit, const QPoint &pos) {
    QMenu menu(this);
    QAction clearAction("Clear Shortcut", &menu);
    connect(&clearAction, &QAction::triggered, lineEdit, [this, &lineEdit]() {
        removeOne(lineEdit->text());
    });
    menu.addAction(&clearAction);
    menu.exec(lineEdit->mapToGlobal(pos));
}

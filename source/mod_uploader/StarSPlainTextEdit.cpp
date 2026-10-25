#include <QPlainTextEdit>

import std;

import star.s_plain_text_edit;

namespace Star {

SPlainTextEdit::SPlainTextEdit(std::optional<std::reference_wrapper<QWidget>> parent)
  : QPlainTextEdit(parent ? &parent->get() : nullptr), m_changed(false) {

  connect(this, SIGNAL(textChanged()), this, SLOT(wasChanged()));
}

void SPlainTextEdit::focusOutEvent(QFocusEvent* e) {
  QPlainTextEdit::focusOutEvent(e);
  m_changed = false;
  emit editingFinished();
}

void SPlainTextEdit::wasChanged() {
  m_changed = true;
}

}

// Generate with moc -i from StarSPlainTextEdit.cppm; compile in this scanned owner.
#include "moc_StarSPlainTextEdit.cpp"

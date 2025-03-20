//2025.03.05-hanpengran edit

#include <QTableView>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QLineEdit>
#include <QDialog>
#include <QListView>
#include <QStringListModel>
#include <QVBoxLayout>
#include <QSortFilterProxyModel>
#include <QAbstractItemView>
#include <QEvent>
#include <QFocusEvent>
#include <QDebug>

class CComplterListView : public QDialog {
    Q_OBJECT
public:
    explicit CComplterListView(QWidget *parent = nullptr)
        : QDialog(parent)
    {
        setWindowFlags(Qt::Tool|Qt::FramelessWindowHint);
        m_listView = new QListView(this);
        m_listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_listViewModel = new QStringListModel(this);
        m_listView->setModel(m_listViewModel);
        connect(m_listView, &QListView::clicked, this, &CComplterListView::onItemClicked);

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setContentsMargins(0,0,0,0);
        layout->addWidget(m_listView);
        setLayout(layout);
    }

    void setQStringList(const QStringList &suggestions) { m_allItems = suggestions; }

    void updateFilter(const QString &text)
    {
        int lastCommaIndex = text.lastIndexOf(',');
        QString suffix = (lastCommaIndex != -1) ? text.mid(lastCommaIndex + 1).trimmed() : text.trimmed();

        if (suffix.isEmpty())
        {
            hide();
            return;
        }

        QStringList suggestions;
        for (const QString &item : m_allItems) {
            if (item.startsWith(suffix, Qt::CaseInsensitive)) {
                suggestions << item;
            }
        }

        m_listViewModel->setStringList(suggestions);
        if (!suggestions.isEmpty()) {
            calculateSize();
            show();
        }
    }
    void calculateSize()
    {

        int rowCount = m_listViewModel->rowCount()+1;
        int totalHeight  = m_listView->sizeHintForRow(0) * rowCount;;
        int maxWidth = 0;
        QFontMetrics metrics(m_listView->font());
        for (const QString &text : m_listViewModel->stringList())
        {
            maxWidth = qMax(maxWidth, metrics.horizontalAdvance(text));
        }
        this->resize(maxWidth,totalHeight);
    }

signals:
    void suggestionClicked(const QString &text);

private slots:
    void onItemClicked(const QModelIndex &index)
    {

        QString selected = m_listView->model()->data(index, Qt::DisplayRole).toString();
        emit suggestionClicked(selected);
        hide();
    }
private:
    QListView *m_listView;
    QStringListModel *m_listViewModel;
    QStringList m_allItems;
};
class CEditComplterDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit CEditComplterDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    { }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */,
                          const QModelIndex &/* index */) const override
    {
        QLineEdit *editor = new QLineEdit(parent);
        m_cComplterListView = new CComplterListView(editor);
        m_cComplterListView->setQStringList(m_stringItems);
        m_cComplterListView->hide();
        connect(editor, &QLineEdit::textChanged, [editor,this](const QString &text)
                {
            QPoint pos = editor->mapToGlobal(QPoint(0, editor->height()));
            m_cComplterListView->move(pos);
            m_cComplterListView->updateFilter(text);
        });

        connect(m_cComplterListView, &CComplterListView::suggestionClicked, [editor, delegate = const_cast<CEditComplterDelegate*>(this)](const QString &text)
                {
            QString itemText = editor->text();
            if(itemText.lastIndexOf(',') != -1)
                itemText = itemText.mid(0,itemText.lastIndexOf(',')+1) + text;
            else
                itemText =text;

            editor->setText(itemText);
            emit delegate->commitData(editor);
            emit delegate->closeEditor(editor);
        });

        return editor;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override
    {
        QString value = index.model()->data(index, Qt::EditRole).toString();
        QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor);
        if (lineEdit)
            lineEdit->setText(value);
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model,const QModelIndex &index) const override
    {
        QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor);
        if (lineEdit)
            model->setData(index, lineEdit->text(), Qt::EditRole);
    }

void setStringItems(const QStringList items)
    {
        m_stringItems = items;
    }
private:
    QStringList m_stringItems;
    mutable CComplterListView *m_cComplterListView;
};

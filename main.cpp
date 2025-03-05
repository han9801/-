
#include <QApplication>
#include <mainwindow.h>
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 构建一个简单的 QTableView，并用 QStandardItemModel 填充一些数据
    QTableView tableView;
    QStandardItemModel model(10, 3);
    for (int row = 0; row < 10; ++row) {
        for (int col = 0; col < 3; ++col) {
            QStandardItem *item = new QStandardItem(QString("Item %1").arg(row * 3 + col));
            model.setItem(row, col, item);
        }
    }
    tableView.setModel(&model);

    // 设置自定义代理
    CEditComplterDelegate *delegate = new CEditComplterDelegate(&tableView);
    QStringList defaultSuggestions;
    defaultSuggestions << "apple" << "apple1" <<"apple2" <<"apple3" <<"apple4" <<"banana" << "orange" << "grape" << "watermelon" << "peach";
    delegate->setStringItems(defaultSuggestions);
    tableView.setItemDelegate(delegate);
    tableView.show();
    return app.exec();
}

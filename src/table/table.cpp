/**
 * \file   table1.cpp
 * \author Aleksey Lekhtin
 * \date   04.11.2015
 * \brief  Таблица с записями
 */

#include <QApplication>
#include <QDate>
#include <QEvent>
#include <QKeyEvent>

#include "dbg.h"
#include "table.h"

//-----------------------------------------------------------
//! Таблица
// \param[in] parent - базовый объект
//
TABLE::TABLE(QWidget *parent) : QTableWidget(parent)
{
    this->setColumnCount(m_labels.size());
    this->setHorizontalHeaderLabels(m_labels);

    this->setSortingEnabled(false);
    this->setSelectionBehavior(QAbstractItemView::SelectRows);
    // this->setSelectionMode(QAbstractItemView::ContiguousSelection);
    // this->setSelectionMode(QAbstractItemView::SingleSelection);
    this->setSelectionMode(QAbstractItemView::ExtendedSelection);
    this->setTabKeyNavigation(false);
    this->setShowGrid(true);

    //-----------------------------------------------------------
    this->horizontalHeader()->setStretchLastSection(true);
    this->horizontalHeader()->setSectionsMovable(true);
    this->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    this->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    //this->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
    this->horizontalHeader()->setDefaultAlignment(Qt::AlignJustify);
    this->horizontalHeader()->setSectionsClickable(true);
    this->horizontalHeader()->setSortIndicatorShown(true);
    //QFont hdr_fnt = this->horizontalHeader()->font();
    this->setHorizontalScrollMode(QAbstractItemView::ScrollPerItem);

    //-----------------------------------------------------------
    int min_row_size = this->verticalHeader()->minimumSectionSize();
    this->verticalHeader()->setDefaultSectionSize(min_row_size);
    this->verticalHeader()->setSectionsMovable(true);
    // this->verticalHeader()->resizeSections(QHeaderView::Fixed);
    this->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    this->verticalHeader()->hide();

    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    // QScrollBar *scroll = this->verticalScrollBar();

    //-----------------------------------------------------------
    connect(this->horizontalHeader(), SIGNAL(sectionClicked(int)),
             this, SLOT(slot_column_clicked(int)));

    connect(this->horizontalHeader(), SIGNAL(sectionResized(int,int,int)),
             this, SLOT(slot_column_resized(int,int,int)));

    connect(this, SIGNAL(itemDoubleClicked(QTableWidgetItem*)),
             this, SLOT(slot_exec_record(QTableWidgetItem*)));

    // connect(m_table, SIGNAL(activated(const QModelIndex &)),
    //          this, SLOT(slot_exec_record(const QModelIndex &)));

    this->installEventFilter(this);
    this->viewport()->installEventFilter(this);
}

//-----------------------------------------------------------
TABLE::~TABLE()
{
    DBG(4, "    ++");
    m_labels.clear();
    DBG(4, "    --");
}

/*----------------------------------------------------------*/
void TABLE::slot_column_clicked(int column)
{
    Q_UNUSED(column);
    // QList<QTableWidgetItem *> list = m_table->selectedItems();
    // if (list.count() > 0)
    //    m_table->scrollToItem(list.at(0), QAbstractItemView::EnsureVisible);
    select_row(currentRow());
}

/*----------------------------------------------------------*/
void TABLE::slot_column_resized(int column_pos, int oldSize, int newSize)
{
    Q_UNUSED(oldSize);
    Q_UNUSED(newSize);
    Q_UNUSED(column_pos);
    ////emit signal_save_settings();
}

//-----------------------------------------------------------
void TABLE::slot_exec_record(const QModelIndex & index)
{
    int nrow = index.row();
    //emit signal__...();
    Q_UNUSED(nrow);
}

//-----------------------------------------------------------
void TABLE::slot_exec_record(QTableWidgetItem *item)
{
    if (!item) return;
    int nrow = item->row();
    //emit signal__...();
    Q_UNUSED(nrow);
}

/*----------------------------------------------------------*/
bool TABLE::eventFilter(QObject *obj, QEvent *ev)
{
    static Qt::KeyboardModifiers s_mod = Qt::NoModifier;

    if (obj != this->viewport())
        return QObject::eventFilter(obj, ev);

    if (ev->type() != QEvent::KeyPress &&
        ev->type() != QEvent::KeyRelease &&
        ev->type() != QEvent::MouseButtonDblClick)
        return QObject::eventFilter(obj, ev);

    int key = 0;
    QKeyEvent *keyEvent = NULL;
    Qt::KeyboardModifiers mod = Qt::NoModifier;
    Qt::MouseButton but = Qt::NoButton;

    if (ev->type() == QEvent::KeyPress)
    {
        keyEvent = static_cast<QKeyEvent*>(ev);
        key = keyEvent->key();
        mod = keyEvent->modifiers();
        s_mod = mod;
    }
    else if (ev->type() == QEvent::KeyRelease)
    {
        s_mod = Qt::NoModifier;
    }
    else if (ev->type() != QEvent::MouseButtonPress)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(ev);
        but = mouseEvent->button();
    }

#if 0 /* debug... */
    bool pressed = false;
    qDebug() << "\n--- keyEvent: " << keyEvent << "\n";

    if (mod == Qt::ControlModifier)
    {
        LOG::dbg2("+++ Control \n");
        qDebug() << mod;
    }

    if (key == Qt::Key_Enter)
    {
        LOG::dbg2("+++ Enter   (0x%x): 0x%x\n", Qt::Key_Enter, key);
        pressed = true;
    }
    else if (key == Qt::Key_Return)
    {
        LOG::dbg2("+++ Return  (0x%x): 0x%x\n", Qt::Key_Return, key);
        pressed = true;
    }

    if (mod == Qt::ControlModifier ||
        mod == (Qt::ControlModifier|Qt::KeypadModifier))
    {
        if (key == Qt::Key_Enter)
            LOG::dbg2("--- Control+Enter\n");

        if (key == Qt::Key_Return)
            LOG::dbg2("--- Control+Return\n");

        if (key == Qt::Key_PageUp)
            LOG::dbg2("--- Control+PageUp\n");

        if (key == Qt::Key_PageDown)
            LOG::dbg2("--- Control+PageDown\n");

        if (key == Qt::Key_Home)
            LOG::dbg2("--- Control+PageHome\n");

        if (key == Qt::Key_End)
            LOG::dbg2("--- Control+PageEnd\n");
    }
    if (pressed) LOG::dbg2("\n");
#endif

    /*----------------------------------------------------------*/
    if (but == Qt::LeftButton)
    {
        // Ctrl + Mouse Left Button
        if (s_mod == Qt::ControlModifier ||
            s_mod == (Qt::ControlModifier|Qt::KeypadModifier))
        {
            s_mod = Qt::NoModifier;
            //slot_...();
            ev->accept();
            return true;
        }
    }
    else if (key == Qt::Key_Enter || key == Qt::Key_Return)
    {
        if (keyEvent->isAutoRepeat()) return true;
        if (mod == Qt::ControlModifier ||
            mod == (Qt::ControlModifier|Qt::KeypadModifier))
        {
            // Ctrl + Enter
            //slot_...();
        }
        else
        {
            // Enter
            slot_exec_record(this->currentIndex());
        }
        ev->accept();
        return true;
    }
    else if (key == Qt::Key_Home)
    {
        if (keyEvent->isAutoRepeat()) return true;
        int row = 0;
        this->clearSelection();
        this->selectRow(row);
        this->scrollToTop();
        //LOG::dbg5("JUMP TOP \n");
        ev->accept();
        return true;
    }
    else if (key == Qt::Key_End)
    {
        if (keyEvent->isAutoRepeat()) return true;
        int row = this->rowCount()-1;
        this->clearSelection();
        this->selectRow(row);
        this->scrollToBottom();
        //LOG::dbg5("JUMP BOTTOM \n");
        ev->accept();
        return true;
    }
    // else if (key == Qt::Key_PageUp)
    // {
    //    // Ctrl+PgUp - jump into '..'
    //    if (mod == Qt::ControlModifier)
    //    {
    //       slot_up_dir();
    //      LOG::dbg5("JUMP DIR UP \n");
    //       ev->accept();
    //       return true;
    //    }
    // }
    // else if (key == Qt::Key_PageDown)
    // {
    //    //  Ctrl+PgDown - jump into dir */
    //    if (mod == Qt::ControlModifier)
    //    {
    //       slot_go_into_dir();
    //       LOG::dbg5("JUMP DIR DOWN \n");
    //       ev->accept();
    //       return true;
    //    }
    // }

    return QObject::eventFilter(obj, ev);
}

//-----------------------------------------------------------
//! Количество записей в таблице
// \return число записей
//
int TABLE::total_records() const
{
    return this->rowCount();
}

//-----------------------------------------------------------
//! Определить список колонок в таблице
// \param labels - описание колонок
// \note Содержание таблицы будет очищено, геометрия будет пересчитана.
//
void TABLE::set_labels(const QStringList &labels)
{
    clear();
    m_labels = labels;
    setColumnCount(m_labels.size());
    setHorizontalHeaderLabels(m_labels);
}

//-----------------------------------------------------------
//! Список колонок в таблице
// \return labels - описание колонок
//
const QStringList & TABLE::get_labels() const
{
    return m_labels;
}

//-----------------------------------------------------------
//! Получить поле из записи таблицы
// \param[in] nrow - номер строки таблицы
// \param[in] ncol - номер колонки таблицы
// \return строка с данными
//
QString TABLE::get_field(int nrow, int ncol) const
{
    if (nrow < 0) return QString();
    //// TableItem *item = static_cast<TableItem*>(this->item(nrow, ncol));
    //// if (item == NULL) return {};
    //// return item->text();

    auto *item = this->item(nrow, ncol);
    if (item == NULL) return {};
    return item->text();
}

QTableWidgetItem *TABLE::get_field_item(int nrow, int ncol) const
{
    if (nrow < 0 || ncol < 0) return nullptr;
    return this->item(nrow, ncol);
}

//-----------------------------------------------------------
//! Задать значение поля в записи таблицы
// \param[in] nrow - номер строки таблицы
// \param[in] ncol - номер колонки таблицы
// \param[in] str  - строка с данными
//
QTableWidgetItem *TABLE::set_field_text(int nrow, int ncol, const QString &text)
{
    //if (nrow < 0 || nrow >= rowCount()) {
    //    return nullptr;
    //}
    //if (ncol < 0 || ncol >= columnCount()) {
    //    return nullptr;
    //}

    auto *item = this->item(nrow, ncol);
    if (!item)
    {
         item = new TableItem(text);
         if (!item) return nullptr;
         item->setText(text);
         item->setToolTip(text);
         setItem(nrow, ncol, item);
         return item;
    }

    item->setText(text);
    //item->setToolTip(text);
    return item;
}

void TABLE::set_field_text(QTableWidgetItem *item, const QString &text)
{
    if (!item) return;
    item->setText(text);
    item->setToolTip(text);
}

//-----------------------------------------------------------
//! Задать выравнивание значения в поле таблицы
// \param[in] nrow - номер строки таблицы
// \param[in] ncol - номер колонки таблицы
// \param[in] align  - тип выравнивания
//
void TABLE::set_field_alignment(int nrow, int ncol, Qt::Alignment align)
{
    if (nrow < 0) return;
    auto *item = this->item(nrow, ncol);
    set_field_alignment(item, align);
}

void TABLE::set_field_alignment(QTableWidgetItem *item, Qt::Alignment align)
{
    if (!item) return;
    item->setTextAlignment(align);
}

//-----------------------------------------------------------
//! Задать цвет поля
// \param[in] nrow - номер строки таблицы
// \param[in] ncol - номер колонки таблицы
// \return строка с данными
//
void TABLE::set_field_color(int nrow, int ncol, QColor foregr, QColor backgr)
{
    if (nrow < 0) return;
    auto *item = this->item(nrow, ncol);
    set_field_color(item, foregr, backgr);
}

void TABLE::set_field_color(QTableWidgetItem *item, QColor foregr, QColor backgr)
{
    if (!item) return;
    item->setBackground(backgr);
    item->setForeground(foregr);

    //// QBrush brush = item->foreground();
    //// brush.setColor(foregr);
    //// brush.setStyle(Qt::SolidPattern);
    //// item->setForeground(brush);

    //// brush = item->background();
    //// brush.setColor(backgr);
    //// brush.setStyle(Qt::SolidPattern);
    //// item->setBackground(brush);
}

void TABLE::set_field_font(int nrow, int ncol, const QFont &fnt)
{
    if (nrow < 0) return;
    auto *item = this->item(nrow, ncol);
    set_field_font(item, fnt);
}

void TABLE::set_field_font(QTableWidgetItem *item, const QFont &fnt)
{
    if (!item) return;
    item->setFont(fnt);
}

void TABLE::set_record_font(int nrow, const QFont &fnt)
{
    if (nrow < 0) return;
    int ncols = columnCount();
    for(int i = 0; i < ncols; ++i)
    {
        auto *item = this->item(nrow, i);
        set_field_font(item, fnt);
    }
}

//-----------------------------------------------------------
//! Получить список выделенных записей таблицы
// \param[out] row_list - список выделенных записей
// \param[out] nrow     - номер первой строки выделения
//
void TABLE::get_selected_list(QList<QModelIndex>& rowlist, int *current_nrow)
{
    if (current_nrow) (*current_nrow) = 0;
    if (this->rowCount() == 0) return;
    QItemSelectionModel *selection = this->selectionModel();
    if (selection == NULL) return;

    rowlist = selection->selectedRows();
    if (rowlist.size() == 0) return;
    if (rowlist.size() == 1)
    {
        int nrow = rowlist.first().row();
        if (current_nrow) (*current_nrow) = nrow;
        return;
    }

    qApp->setOverrideCursor(Qt::WaitCursor);
    //// qStableSort(row_list);
    std::stable_sort(rowlist.begin(), rowlist.end());
    int nrow = rowlist.first().row();
    if (current_nrow) (*current_nrow) = nrow;

    //qApp->processEvents();
    qApp->restoreOverrideCursor();
}

//-----------------------------------------------------------
//! Снять выделение со строк таблицы
// \param[out] nrow - номер строки таблицы
//
void TABLE::deselect_row(int nrow)
{
    QItemSelectionModel *selection = this->selectionModel();
    if (selection == NULL) return;

    QAbstractItemModel *md = this->model();
    if (md != NULL)
    {
        for(int ncol = 0; ncol < m_labels.size(); ++ncol)
        {
            selection->select(md->index(nrow, ncol, QModelIndex()),
                               QItemSelectionModel::Deselect);
            //QItemSelectionModel::Current);
        }
    }
}

//-----------------------------------------------------------
//! Получить номер текущей записи
// \return номер текущей строки таблицы
//
int TABLE::get_current_num() const
{
    if (rowCount() == 0) return -1;
    return currentRow();
}

//-----------------------------------------------------------
//! Установить признак 'отмечено'
// \param[in] nrow - номер строки таблицы
// \param[in] mark - да / нет
//
void TABLE::set_record_mark(int nrow, bool mark)
{
    if (nrow < 0) return;
    auto *item = static_cast<TableItem*>(this->item(nrow, 0));
    if (item) item->m_mark = mark;
}

//-----------------------------------------------------------
//! Узнать состояние признака 'отмечено'
// \param[in] nrow - номер строки таблицы
// \return да / нет
//
bool TABLE::get_record_mark(int nrow) const
{
    if (nrow < 0) return false;
    auto *item = static_cast<TableItem*>(this->item(nrow, 0));
    if (!item) return false;
    return item->m_mark;
}

//-----------------------------------------------------------
//! Перейти на строку таблицы
//
void TABLE::select_row(int nrow)
{
    //if (!this->isVisible()) return;
    if (nrow < 0) return;
    nrow = qMin(nrow, this->rowCount()-1);
    this->clearSelection();
    if (nrow < 0) return;
    QTableWidgetItem *new_item = this->item(nrow, 0);
    this->setCurrentItem(new_item);
    this->scrollToItem(new_item, QAbstractItemView::EnsureVisible);
    selectRow(nrow);
    /* QAbstractItemView::PositionAtCenter */
}

//-----------------------------------------------------------
//! Изменить запись в таблице
// \param[in] data - список данных для отображения в строке таблицы
// \return номер измененной записи
//
int TABLE::set_record(int nrow, const QStringList &data, QList<Qt::Alignment> align, const QString &tip)
{
    QTableWidgetItem it;
    QFont fnt = font();
    fnt = it.font();
    fnt.setBold(true);

    int ncols = columnCount();
    for(int i = 0; i < ncols; ++i)
    {
        QString buf;
        if (i < data.size()) buf = data.at(i);

        TableItem *item = new TableItem();
        item->setText(buf);

        if (!tip.isEmpty())
            item->setToolTip(buf);
        else
            item->setToolTip(tip);

        if (!align.isEmpty() && i < align.size())
            item->setTextAlignment(align[i]);
        else
            item->setTextAlignment(Qt::AlignCenter);

        item->setFont(fnt);
        setItem(nrow, i, item);
    }

    return nrow;
}

//-----------------------------------------------------------
//! Добавить запись в таблицу
// \param[in] data - список данных для отображения в строке таблицы
// \return номер новой записи
//
int TABLE::add_record(const QStringList &data)
{
    int nrow = rowCount();
    insertRow(nrow);
    if (data.size() == 0)
    {
        for(int ncol = 0; ncol < columnCount(); ++ncol)
        {
            setItem(nrow, ncol, new TableItem());
        }
        return nrow;
    }

    TableItem *item;
    auto num = data.size();
    for(int ncol = 0; ncol < columnCount() && ncol < num; ++ncol)
    {
        item = new TableItem();
        item->setText(data.at(ncol));
        item->setToolTip(data.at(ncol));
        setItem(nrow, ncol, item);
    }
    return nrow;
}

//-----------------------------------------------------------
//! Удалить запись таблицы
// \param[in] nrow - номер удаляемой строки таблицы
//
void TABLE::del_record(int nrow)
{
    if (nrow < 0) return;
    if (nrow > rowCount()-1) return;
    removeRow(nrow);
    //select_rec(nrow);
}

//-----------------------------------------------------------
//! Удалить несколько записей таблицы
// \param[in] row_list - список удаляемых строк таблицы
//
void TABLE::del_records(const QList<QModelIndex> & row_list)
{
    QList<QModelIndex> rowlist = row_list;
    //// qStableSort(rowlist);
    std::stable_sort(rowlist.begin(), rowlist.end());
    int nrow = rowlist.first().row();
    int cnt = rowlist.size();
    if (cnt == 0)
    {
        del_record(nrow);
        return;
    }

    QList<QModelIndex>::const_reverse_iterator it;
    it = rowlist.rbegin();
    while(it != rowlist.rend())
    {
        const QModelIndex & md = *it;
        //qDebug() << md.row();
        removeRow(md.row());
        ++it;
    }
    //qDebug() << "selected:" << nrow;
    //select_rec(nrow);
}

//-----------------------------------------------------------
//! Удалить несколько записей таблицы
// \param[in] row_list - список номеров удаляемых строк таблицы
//
void TABLE::del_records(const QList<int> & row_list)
{
    QList<int> rowlist = row_list;
    //// qStableSort(rowlist);
    std::stable_sort(rowlist.begin(), rowlist.end());
    int nrow = rowlist.first();
    int cnt = rowlist.size();
    if (cnt == 0)
    {
        del_record(nrow);
        return;
    }

    QList<int>::const_reverse_iterator it;
    it = rowlist.rbegin();
    while(it != rowlist.rend())
    {
        //qDebug() << (*it);
        removeRow((*it));
        ++it;
    }
    //qDebug() << "selected:" << nrow;
    //select_rec(nrow);
}

//-----------------------------------------------------------
QString TABLE::date_to_yyyymmdd(const QString & date_yyyy_mm_dd) const
{
    QString buf;
    QDate dt = QDate::fromString(date_yyyy_mm_dd, "yyyy-MM-dd");
    if (!dt.isValid()) return date_yyyy_mm_dd;
    //// QLocale lo;
    if (locale().language() == QLocale::Russian ||
        locale().country()  == QLocale::RussianFederation)
        buf = dt.toString("dd.MM.yyyy");
    else
        buf = dt.toString(Qt::TextDate);
    return buf;
}

//--- Constructor ------------------------------------------
TableItem::TableItem(bool date_rus_format) :
    m_date_rus(date_rus_format),
    m_mark(false)
{
    Qt::ItemFlags flag = flags();
    flag &= (~Qt::ItemIsEditable);
    setFlags(flag);
}

TableItem::TableItem(const QString &text, bool date_rus_format) :
    TableItem(date_rus_format)
{
     if (!text.isNull()) setText(text);
}


//-----------------------------------------------------------
bool TableItem::operator<(const QTableWidgetItem & other) const
{
    bool rc = false;
    //QTableWidget *tbl = tableWidget();
    //Qt::SortOrder sort_order = tbl->horizontalHeader()->sortIndicatorOrder();
    //int sort_ncol  = tbl->horizontalHeader()->sortIndicatorSection();

    TableItem & item2 = (TableItem &)other;
    const QVariant d1 = data(Qt::DisplayRole);
    const QVariant d2 = item2.data(Qt::DisplayRole);

    QString str1 = d1.toString().trimmed();
    QString str2 = d2.toString().trimmed();

    //-----------------------------------------------------------
    // if (sort_ncol == COL_DATE)
    // {
    //    QDate dt1;
    //    QDate dt2;
    //    if (m_date_rus == true)
    //    {
    //       dt1 = QDate::fromString(str1, "dd.MM.yyyy");
    //       dt2 = QDate::fromString(str2, "dd.MM.yyyy");
    //    }
    //    else
    //    {
    //       dt1 = QDate::fromString(str1);
    //       dt2 = QDate::fromString(str2);
    //    }

    //    if (str1.isEmpty())
    //    {
    //       rc = true;
    //       if (str2.isEmpty()) rc = (sort_order ? false : true);
    //       return rc;
    //    }
    //    else if (str2.isEmpty())
    //    {
    //       return false;
    //    }

    //    rc = (dt1 >= dt2);
    //    return !rc;
    // }

    //-----------------------------------------------------------
    //switch(d1.metaType().id())
    switch(d1.typeId())
    {
    case QMetaType::QString:
        rc = str1.compare(str2, Qt::CaseInsensitive) < 0;
        break;
    case QMetaType::Int:
        rc = d1.toInt() < d2.toInt(); break;
    case QMetaType::UInt:
        rc = d1.toUInt() < d2.toUInt(); break;
    case QMetaType::LongLong:
        rc = d1.toLongLong() < d2.toLongLong(); break;
    case QMetaType::ULongLong:
        rc = d1.toULongLong() < d2.toULongLong(); break;
    case QMetaType::Double:
        rc = d1.toDouble() < d2.toDouble(); break;
    case QMetaType::QChar:
        rc = d1.toChar() < d2.toChar(); break;
    case QMetaType::QDate:
        rc = d1.toDate() > d2.toDate(); break;
    case QMetaType::QTime:
        rc = d1.toTime() < d2.toTime(); break;
    case QMetaType::QDateTime:
        rc = d1.toDateTime() < d2.toDateTime(); break;
    case QMetaType::UnknownType:
        //rc = (d2.metaType().id() == QMetaType::UnknownType); break;
        rc = (d2.typeId() == QVariant::Invalid); break;
    default:
        rc = str1.compare(str2, Qt::CaseInsensitive) < 0; break;
    }
    return !rc;
}

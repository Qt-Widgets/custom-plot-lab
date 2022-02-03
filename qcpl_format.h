#ifndef QCPL_FORMAT_H
#define QCPL_FORMAT_H

#include <QVector>
#include <QSize>

#include "qcpl_types.h"

class QCPAxis;
class QCPRange;

namespace QCPL {

class TextFormatterBase;

struct AxisTitleDlgProps
{
    QString title;
    QSize iconSize;
    TextFormatterBase *formatter = nullptr;
};

bool axisTitleDlg(QCPAxis* axis, const AxisTitleDlgProps& props);

struct AxisLimitsDlgProps
{
    QString title;
    QString unit;
    int precision = 6;
};

bool axisLimitsDlg(QCPRange& range, const AxisLimitsDlgProps& props);

class TextProcessor
{
public:
    void addVar(const QString& name, TextVarGetter getter);

    QString process(const QString& text) const;

private:
    struct Var
    {
        QString name;
        TextVarGetter getter;
    };

    QVector<Var> _vars;
};

class TextFormatterBase
{
public:
    virtual ~TextFormatterBase() {}

    void addVar(const QString& name, const QString& descr, TextVarGetter getter);
    const QVector<TextVariable>& vars() const { return _vars; }
    virtual void format() = 0;

    QString text() const { return _text; };
    void setText(const QString& text) { _text = text; }

protected:
    QString _text;
    TextProcessor _processor;
    QVector<TextVariable> _vars;
};

class AxisTitleFormatter : public TextFormatterBase
{
public:
    AxisTitleFormatter(QCPAxis* axis);

    void format() override;

private:
    QCPAxis *_axis;
};

} // namespace QCPL

#endif // QCPL_FORMAT_H

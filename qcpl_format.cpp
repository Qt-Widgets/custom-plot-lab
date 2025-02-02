#include "qcpl_format.h"
#include "qcpl_plot.h"
#include "qcpl_text_editor.h"
#include "qcpl_format_axis.h"
#include "qcpl_format_plot.h"

#include "helpers/OriDialogs.h"
#include "helpers/OriWidgets.h"
#include "widgets/OriValueEdit.h"

namespace QCPL {

bool axisTitleDlg(QCPAxis* axis, const AxisTitleDlgProps& props)
{
    auto style = qApp->style();

    TextEditorWidget::Options opts;
    opts.iconSize = props.iconSize;
    if (props.formatter)
        opts.vars = props.formatter->vars();
    TextEditorWidget editor(opts);
    if (props.formatter)
        editor.setText(props.formatter->text());
    else
        editor.setText(axis->label());
    editor.setFont(axis->labelFont());
    editor.setColor(axis->labelColor());
    editor.setContentsMargins(style->pixelMetric(QStyle::PM_LayoutLeftMargin)/2, 0,
                              style->pixelMetric(QStyle::PM_LayoutRightMargin)/2, 0);

    if (Ori::Dlg::Dialog(&editor, false)
            .withTitle(props.title)
            .withSkipContentMargins()
            .withContentToButtonsSpacingFactor(2)
            .withPersistenceId("axis-title")
            .withAcceptSignal(SIGNAL(acceptRequested()))
            .withActiveWidget(editor.editor())
            .exec())
    {
        if (props.formatter)
        {
            props.formatter->setText(editor.text());
            props.formatter->format();
        }
        else
            axis->setLabel(editor.text());
        axis->setLabelFont(editor.font());
        axis->setLabelColor(editor.color());
        axis->setSelectedLabelFont(editor.font());
        return true;
    }
    return false;
}

bool axisTitleDlgV2(QCPAxis* axis, const AxisTitleDlgPropsV2& props)
{
    auto style = qApp->style();

    TextEditorWidgetV2::Options opts;
    opts.defaultText = props.defaultTitle;
    if (props.formatter)
        opts.vars = props.formatter->vars();
    TextEditorWidgetV2 editor(opts);
    if (props.formatter)
        editor.setText(props.formatter->text());
    else
        editor.setText(axis->label());
    editor.setContentsMargins(style->pixelMetric(QStyle::PM_LayoutLeftMargin)/2,
                              style->pixelMetric(QStyle::PM_LayoutTopMargin)/2,
                              style->pixelMetric(QStyle::PM_LayoutRightMargin)/2, 0);

    if (Ori::Dlg::Dialog(&editor, false)
            .withTitle(props.title)
            .withSkipContentMargins()
            .withContentToButtonsSpacingFactor(2)
            .withPersistenceId("axis-title-v2")
            .withAcceptSignal(SIGNAL(acceptRequested()))
            .withActiveWidget(editor.editor())
            .exec())
    {
        if (props.formatter)
        {
            props.formatter->setText(editor.text());
            props.formatter->format();
        }
        else
            axis->setLabel(editor.text());
        return true;
    }
    return false;
}

bool axisLimitsDlg(QCPRange& range, const AxisLimitsDlgProps& props)
{
    auto editorMin = new Ori::Widgets::ValueEdit;
    auto editorMax = new Ori::Widgets::ValueEdit;
    Ori::Gui::adjustFont(editorMin);
    Ori::Gui::adjustFont(editorMax);
    editorMin->setNumberPrecision(props.precision);
    editorMax->setNumberPrecision(props.precision);
    editorMin->setValue(range.lower);
    editorMax->setValue(range.upper);
    editorMin->selectAll();

    QWidget w;

    auto layout = new QFormLayout(&w);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addRow(new QLabel(props.unit.isEmpty() ? QString("Min") : QString("Min (%1)").arg(props.unit)), editorMin);
    layout->addRow(new QLabel(props.unit.isEmpty() ? QString("Max") : QString("Max (%1)").arg(props.unit)), editorMax);

    if (Ori::Dlg::Dialog(&w, false)
            .withTitle(props.title)
            .withContentToButtonsSpacingFactor(3)
            .exec())
    {
        range.lower = editorMin->value();
        range.upper = editorMax->value();
        range.normalize();
        return true;
    }
    return false;
}

bool axisFormatDlg(QCPAxis* axis, const AxisFormatDlgProps& props)
{
    AxisFormatWidget editor(axis);

    return Ori::Dlg::Dialog(&editor, false)
            .withTitle(props.title)
            .withOnApply([&editor, &props]{ editor.apply(); props.plot->replot(); })
            .connectOkToContentApply()
            .exec();
}

bool plotFormatDlg(Plot* plot, const PlotFormatDlgProps &props)
{
    PlotFormatWidget editor(plot, {});

    return Ori::Dlg::Dialog(&editor, false)
            .withTitle(props.title.isEmpty() ? "Plot Format" : props.title)
            .withOnApply([&editor, plot]{ editor.apply(); plot->replot(); })
            .connectOkToContentApply()
            .exec();
}

//------------------------------------------------------------------------------
//                            QCPL::TextProcessor
//------------------------------------------------------------------------------

void TextProcessor::addVar(const QString& name, TextVarGetter getter)
{
    _vars.append({name, getter});
}

QString TextProcessor::process(const QString& text) const
{
    QString str(text);
    foreach (const auto& var, _vars)
    {
        int pos = str.indexOf(var.name);
        if (pos >= 0)
        {
            QString value = var.getter();
            while (pos >= 0)
            {
                str.replace(pos, var.name.length(), value);
                pos = str.indexOf(var.name, pos + value.length());
            }
        }
    }
    return str;
}

//------------------------------------------------------------------------------
//                           QCPL::TextFormatterBase
//------------------------------------------------------------------------------

void TextFormatterBase::addVar(const QString& name, const QString& descr, TextVarGetter getter)
{
    _vars.append({name, descr});
    _processor.addVar(name, getter);
}

//------------------------------------------------------------------------------
//                          QCPL::AxisTitleFormatter
//------------------------------------------------------------------------------

AxisTitleFormatter::AxisTitleFormatter(QCPAxis* axis): _axis(axis)
{
}

void AxisTitleFormatter::format()
{
    _axis->setLabel(_processor.process(_text).trimmed());
}

} // namespace QCPL

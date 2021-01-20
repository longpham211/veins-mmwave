#!/usr/bin/env Rscript

# install.packages ('ggplot2')
library(ggplot2)
library(ggpubr)

## # install.packages ('scales')
## library (scales)

args = commandArgs(trailingOnly=TRUE)
#data = read.table(args[1], header=T, sep=";")
# data100 = read.table("/home/longpham211/Workspace/thesis/veins_simulator/src/veins-mmwave/examples/veins-mmwave/scripts/100mResult/aggregateDataUrban100BI100.csv", header=T, sep=";")
# data60 = read.table("/home/longpham211/Workspace/thesis/veins_simulator/src/veins-mmwave/examples/veins-mmwave/scripts/100mResult/aggregateDataUrban100BI60.csv", header=T, sep=";")
# data30 = read.table("/home/longpham211/Workspace/thesis/veins_simulator/src/veins-mmwave/examples/veins-mmwave/scripts/100mResult/aggregateDataUrban100BI30.csv", header=T, sep=";")

data100 = read.table(args[1], header=T, sep=";")
data60 = read.table(args[2], header=T, sep=";")
data30 = read.table(args[3], header=T, sep=";")

data100$Timeslot =  factor(data100$Timeslot, levels = c("1 a.m.","2 a.m.","3 a.m.","4 a.m.","5 a.m.","6 a.m.","7 a.m.","8 a.m.","9 a.m.","10 a.m.","11 a.m.","12 p.m.",
                                                        "1 p.m.","2 p.m.","3 p.m.","4 p.m.","5 p.m.","6 p.m.","7 p.m.","8 p.m.","9 p.m.","10 p.m.","11 p.m.","12 a.m."))
data60$Timeslot =  factor(data60$Timeslot, levels = c("1 a.m.","2 a.m.","3 a.m.","4 a.m.","5 a.m.","6 a.m.","7 a.m.","8 a.m.","9 a.m.","10 a.m.","11 a.m.","12 p.m.",
                                                        "1 p.m.","2 p.m.","3 p.m.","4 p.m.","5 p.m.","6 p.m.","7 p.m.","8 p.m.","9 p.m.","10 p.m.","11 p.m.","12 a.m."))
data30$Timeslot =  factor(data30$Timeslot, levels = c("1 a.m.","2 a.m.","3 a.m.","4 a.m.","5 a.m.","6 a.m.","7 a.m.","8 a.m.","9 a.m.","10 a.m.","11 a.m.","12 p.m.",
                                                        "1 p.m.","2 p.m.","3 p.m.","4 p.m.","5 p.m.","6 p.m.","7 p.m.","8 p.m.","9 p.m.","10 p.m.","11 p.m.","12 a.m."))

p100 <- ggplot(data100, aes(x=Timeslot, y=Mean, colour=Algorithm, group=Algorithm)) +
  geom_line(aes(color=Algorithm), size=0.3) +
  geom_point(aes(shape=Algorithm),size=1) +
  scale_x_discrete(breaks=c("1 a.m.", "5 a.m.","9 a.m.","1 p.m.","5 p.m.","9 p.m.")) +
  # scale_y_continuous(limits = c(0, 15000), expand = c(0, 0)) +
  ylim(0, 16500) +
  scale_color_manual(name  ="Algorithm",
                     values=c("#04B486","#2E9AFE", "#DBA901", "#5F5C58","#E95E3F", "#7E6148B2", "#F39B7FB2", "#3C5488B2", "#50105A", "#D2E3F5"))+
  scale_shape_discrete(name  ="Algorithm")+
  ylab("Aggregate data per hour (Gb)") +
  xlab("(a) Beacon Interval: 100 ms") +
  theme(legend.position="none", 
        axis.title.y = element_text(size=12, family="Times"),
        legend.title = element_blank())

p60 <- ggplot(data60, aes(x=Timeslot, y=Mean, colour=Algorithm, group=Algorithm)) +
  geom_line(aes(color=Algorithm), size=0.3) +
  geom_point(aes(shape=Algorithm),size=1) +
  ylim(0, 16500) + 
  scale_x_discrete(breaks=c("1 a.m.", "5 a.m.","9 a.m.","1 p.m.","5 p.m.","9 p.m.")) +
  scale_color_manual(name  ="Algorithm",
                     values=c("#04B486","#2E9AFE", "#DBA901", "#5F5C58","#E95E3F", "#7E6148B2", "#F39B7FB2", "#3C5488B2", "#50105A", "#D2E3F5"))+
  scale_shape_discrete(name  ="Algorithm")+
  ylab("Aggregate data per hour (Gb)") +
  xlab("(b) Beacon Interval: 60 ms") + 
  theme(legend.position="none", 
        axis.title.y = element_text(size=12, family="Times"),
        legend.title = element_blank())

p30 <- ggplot(data30, aes(x=Timeslot, y=Mean, colour=Algorithm, group=Algorithm)) +
  geom_line(aes(color=Algorithm), size=0.3) +
  geom_point(aes(shape=Algorithm),size=1) +
  ylim(0, 16500) + 
  scale_x_discrete(breaks=c("1 a.m.", "5 a.m.","9 a.m.","1 p.m.","5 p.m.","9 p.m.")) +
  scale_color_manual(name  ="Algorithm",
                     values=c("#04B486","#2E9AFE", "#DBA901", "#5F5C58","#E95E3F", "#7E6148B2", "#F39B7FB2", "#3C5488B2", "#50105A", "#D2E3F5"))+
  scale_shape_discrete(name  ="Algorithm")+
  ylab("Aggregate data per hour (Gb)") +
  xlab("(c) Beacon Interval: 30 ms") + 
  theme(legend.position="none", 
        axis.title.y = element_text(size=12, family="Times"),
        legend.title = element_blank())

# https://stackoverflow.com/questions/13649473/add-a-common-legend-for-combined-ggplots
p <- ggarrange(p100, p60, p30, nrow=1, ncol=3, common.legend = TRUE, legend="bottom", heights = c(10,2.75)) 
p <- annotate_figure(p, top = text_grob("Aggregate data per hour (Gb)", face = "bold", size = 14))

# ggexport(p, filename="/home/seemoo/lpham-thesis/src/veins-mmwave/examples/veins-mmwave/scripts/100mResult/aggregateDataUrban100.pdf")
ggsave(file="/home/longpham211/Workspace/thesis/veins_simulator/src/veins-mmwave/presentation/aggregateData.pdf", p, width = 10, height = 3.75, device=cairo_pdf)
#ggsave(file=args[4], p, device=cairo_pdf)
x = dev.off()

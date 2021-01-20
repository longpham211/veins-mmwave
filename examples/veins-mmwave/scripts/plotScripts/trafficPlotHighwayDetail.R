#!/usr/bin/env Rscript

# install.packages ('ggplot2')
library(ggplot2)
library(ggpubr)
library(gridExtra)
library(grid)

args = commandArgs(trailingOnly=TRUE)
dataN2S = read.table("/home/longpham211/Workspace/thesis/veins_simulator/src/veins-mmwave/examples/veins-mmwave/routeHighwayPlotN2S.txt", header=T, stringsAsFactors=F, sep=";")
dataS2N = read.table("/home/longpham211/Workspace/thesis/veins_simulator/src/veins-mmwave/examples/veins-mmwave/routeHighwayPlotS2N.txt", header=T, stringsAsFactors=F, sep=";")
dataN2S$Hour = factor(dataN2S$Hour, levels = c("1 a.m.","2 a.m.","3 a.m.","4 a.m.","5 a.m.","6 a.m.","7 a.m.","8 a.m.","9 a.m.","10 a.m.","11 a.m.","12 p.m.",
                                         "1 p.m.","2 p.m.","3 p.m.","4 p.m.","5 p.m.","6 p.m.","7 p.m.","8 p.m.","9 p.m.","10 p.m.","11 p.m.","12 a.m."))
dataS2N$Hour = factor(dataS2N$Hour, levels = c("1 a.m.","2 a.m.","3 a.m.","4 a.m.","5 a.m.","6 a.m.","7 a.m.","8 a.m.","9 a.m.","10 a.m.","11 a.m.","12 p.m.",
                                         "1 p.m.","2 p.m.","3 p.m.","4 p.m.","5 p.m.","6 p.m.","7 p.m.","8 p.m.","9 p.m.","10 p.m.","11 p.m.","12 a.m."))

pN2S <- ggplot(dataN2S, aes(x=Hour, y=Count)) +
  geom_col(aes(fill = VehicleType), width = 0.7) + 
  scale_x_discrete(breaks=c("1 a.m.", "5 a.m.","9 a.m.","1 p.m.","5 p.m.","9 p.m.")) +
  scale_fill_manual(values=c("#04B486","#2E9AFE", "#DBA901","#E95E3F", "#5F5C58", "#7E6148B2", "#F39B7FB2", "#3C5488B2", "#50105A", "#D2E3F5"))+
  #labs(fill = "Types of vehicle") + 
  ylab("Number of vehicles") +
  xlab("Blue - North to South traffic flow") +
  theme(legend.position="none", 
        axis.title.y = element_text(size=12, family="Times"),
        legend.title = element_blank())

pS2N <- ggplot(dataS2N, aes(x=Hour, y=Count)) +
  geom_col(aes(fill = VehicleType), width = 0.7) + 
  scale_x_discrete(breaks=c("1 a.m.", "5 a.m.","9 a.m.","1 p.m.","5 p.m.","9 p.m.")) +
  scale_fill_manual(values=c("#04B486","#2E9AFE", "#DBA901","#E95E3F", "#5F5C58", "#7E6148B2", "#F39B7FB2", "#3C5488B2", "#50105A", "#D2E3F5"))+
  #labs(fill = "Types of vehicle") + 
  ylab("Number of vehicles") +
  xlab("Green - South to North traffic flow") +
  theme(legend.position="bottom", 
        axis.title.y = element_text(size=12, family="Times"),
        legend.title = element_blank(),
        legend.text = element_text(size = 7))

# #https://github.com/hadley/ggplot2/wiki/Share-a-legend-between-two-ggplot2-graphs
get_legend<-function(a.gplot){
  tmp <- ggplot_gtable(ggplot_build(a.gplot))
  leg <- which(sapply(tmp$grobs, function(x) x$name) == "guide-box")
  legend <- tmp$grobs[[leg]]
  return(legend)}

# https://stackoverflow.com/questions/13649473/add-a-common-legend-for-combined-ggplots
#p <- ggarrange(pN2S, pS2N, nrow=1, ncol=2, common.legend = TRUE, legend="bottom", heights = c(3.75,3.75)) 
#p <- annotate_figure(p, top = text_grob("Aggregate data per hour (Gb)", face = "bold", size = 14))

# https://stackoverflow.com/questions/59162865/how-to-edit-common-legend-title-in-ggarrange
p2_legend <- get_legend(pS2N)
p <- grid.arrange(arrangeGrob(pN2S + theme(legend.position="none"),
                         pS2N + theme(legend.position="none"), nrow=1, ncol=2),
                         p2_legend,
                         nrow=2,heights=c(10, 1))

# ggexport(p, filename="/home/seemoo/lpham-thesis/src/veins-mmwave/examples/veins-mmwave/scripts/100mResult/aggregateDataUrban100.pdf")
#ggsave(file="/home/longpham211/Workspace/thesis/veins_simulator/src/veins-mmwave/examples/veins-mmwave/scripts/100mResult/aggregateDataUrban100.pdf", p)

ggsave(file="/home/longpham211/Workspace/thesis/veins_simulator/src/veins-mmwave/writting/seemoo-thesis-template/gfx/plots/HighwayTrafficDetail.pdf", p, device=cairo_pdf)
x = dev.off()

# g.all<- ggplot(data, aes(x=Timeslot, y=Mean, colour=Algorithm, group=Algorithm)) +
#   geom_errorbar(aes(ymin=Mean-SD, ymax=Mean+SD), width=.2,
#                 position=position_dodge(0.0))+
#   geom_line(aes(color=Algorithm), size=1) +
#   geom_point(aes(shape=Algorithm),size=3) +
#   scale_x_discrete(breaks=c("1 a.m.", "5 a.m.","9 a.m.","1 p.m.","5 p.m.","9 p.m.")) +
#   scale_color_manual(name  ="Algorithm", 
#                      values=c("#04B486","#2E9AFE", "#DBA901", "#5F5C58","#E95E3F", "#7E6148B2", "#F39B7FB2", "#3C5488B2", "#50105A", "#D2E3F5"))+
#   scale_shape_discrete(name  ="Algorithm")+
#   ggtitle("Aggregate Data per Hour")+
# #  xlab("Hour")+
#   ylab("Aggregate Data (Gb)")+
#   theme(plot.title = element_text(lineheight=.8, size=20, family="Times", face="bold", hjust=0.5),
#         #axis.title.x = element_text(size=18, family="Times"),
#         axis.title.x = element_blank(),
#         axis.title.y = element_text(size=18, family="Times"),
#         axis.text.x  = element_text(vjust=0.35, size=18, family="Times"),
#         axis.text.y  = element_text(vjust=0.35, size=18, family="Times"),
#         legend.title= element_text(size=16, face="bold", family="Times"),
#         legend.background = element_rect(colour = "black"),
#         legend.key.size = unit(1, "cm"),
#         legend.text = element_text(size = 18, family="Times"))+
#   # Uncomment this line for setting the position of legend
#   # legend.position=c(.2, 0.4))+
#   ggsave(file=args[2], width=8, height=6)
#   #ggsave(file="/home/seemoo/lpham-thesis/src/veins-mmwave/examples/veins-mmwave/results/aggregateData.pdf", width=8, height=6)
#   print(g.all)
#  x = dev.off()

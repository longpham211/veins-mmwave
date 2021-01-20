#!/usr/bin/env Rscript

# install.packages ('ggplot2')
library(ggplot2)
## # install.packages ('scales')
## library (scales)

args = commandArgs(trailingOnly=TRUE)
#data = read.table(args[1], header=T, sep=";")
data = read.table("/home/longpham211/Workspace/thesis/veins_simulator/src/veins-mmwave/examples/veins-mmwave/routeHighwayPlotAll.txt", header=T, stringsAsFactors=F, sep=";")


data$Flow = factor(data$Flow)
data$Hour = factor(data$Hour, levels = c("1 a.m.","2 a.m.","3 a.m.","4 a.m.","5 a.m.","6 a.m.","7 a.m.","8 a.m.","9 a.m.","10 a.m.","11 a.m.","12 p.m.",
                                         "1 p.m.","2 p.m.","3 p.m.","4 p.m.","5 p.m.","6 p.m.","7 p.m.","8 p.m.","9 p.m.","10 p.m.","11 p.m.","12 a.m."))
data$VehicleType = data$VehicleType
data$Count = data$Count

dodge <- position_dodge(width=2)
pd <- position_dodge(0.0)

#data
#data$Hour

g.all<- ggplot(data, aes(x=Hour, y=Count, colour=Flow, group=Flow)) +
  #geom_errorbar(aes(ymin=Mean-SD, ymax=Mean+SD), width=.2,
   #             position=position_dodge(0.0))+
  #scale_y_log10() +
  geom_line(aes(color=Flow), size=1,) +
  geom_point(aes(shape=Flow),size=2,) +
  scale_x_discrete(breaks=c("1 a.m.", "5 a.m.","9 a.m.","1 p.m.","5 p.m.","9 p.m.")) +
  #xlim(0, 25) +
  #scale_fill_brewer(palette="Blues")+
  scale_color_manual(name  ="Traffic Flow", 
                     #values=c("#04B486","#2E9AFE", "#DBA901", "#5F5C58","#E95E3F"))+
                     values=c("#0000FF", "#00FF00", "#CD853F"))+
  scale_shape_discrete(name  ="Traffic Flow")+
  #ggtitle("Urban traffic")+
  #xlab("")+
  ylab("Number of vehicles")+
  theme(plot.title = element_text(lineheight=.8, size=20, family="Times", face="bold", hjust=0.5),
        #axis.title.x = element_text(size=18, family="Times"),
        axis.title.x = element_blank(),
        axis.title.y = element_text(size=18, family="Times"),
        axis.text.x  = element_text(vjust=0.35, size=18, family="Times"),
        axis.text.y  = element_text(vjust=0.35, size=18, family="Times"),
        legend.title= element_text(size=16, face="bold", family="Times"),
        legend.background = element_rect(colour = "black"),
        legend.key.size = unit(1, "cm"),
        legend.text = element_text(size = 18, family="Times"))
  # Uncomment this line for setting the position of legend
  # legend.position=c(.2, 0.4))+
  # ggsave(file=args[2], width=8, height=6)
  ggsave(file="/home/longpham211/Workspace/thesis/veins_simulator/src/veins-mmwave/writting/seemoo-thesis-template/gfx/plots/HighwayTraffic.pdf", width=8, height=6, plot = g.all, device=cairo_pdf)
  #print(g.all)
 #x = dev.off()
package Dnmap;

import java.io.BufferedReader;
import java.io.FileReader;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

public class B {

	private List<Bean> bList;
	private String[] string;
	private List<Mysql> list = new ArrayList<Mysql>(280000);

	public B(List<Bean> bList) {
		this.bList = bList;
	}

	private void start(String path) throws Exception {
		BufferedReader reader = new BufferedReader(new FileReader(path));
		while (reader.ready()) {
			String string = reader.readLine();
			String str[] = string.split(" ");
			String id = str[0];
			String produce = str[1];
			list.add(new Mysql(id, produce));
		}
		Collections.sort(list);
		string = new String[list.size()];
		for (int i = 0; i < list.size(); i++)
			string[i] = list.get(i).getProduce();
		reader.close();
	}

	public StringBuilder init() {
		try {
	start("/home/software/sourceData.txt");
//			start("D:/sourceData.txt");
		} catch (Exception e) {
		}
		String id = "";
		StringBuilder builder = new StringBuilder();
		for (Bean bean : bList) {
			builder.append(bean.getIp() + "\n");
			builder.append(bean.getPort() + "\n");
			builder.append(bean.getCity() + "\n");
			builder.append(bean.getLongitude() + "\n");
			builder.append(bean.getLatitude() + "\n");
			builder.append(bean.getOS() + "\n");
			builder.append(bean.getDeviceType() + "\n");
			builder.append(bean.getServer() + "\n");
			String string = contain(bean.getProduct());
			if (!string.equals(""))
				id = string;
			string = add(bean.getOS());
			if (!string.equals(""))
				id += string;
			builder.append("Â©¶´ÐÅÏ¢£º" + id + "\n\n");
			id = "";
		}
		return builder;
	}

	private String add(String key) {
		String str[] = key.split("£º");
		if (str.length == 2) {
			String os = str[1];
			int s = Arrays.binarySearch(string, os);
			if (s > 0)
				return list.get(s).getId();
		}
		return "";
	}

	
	private String contain(String line) {
		String str[] = line.split("£º");
		if (str.length == 2) {
			String produce = str[1];
			for (Mysql mysql : list)
				if (mysql.getProduce().contains(produce))
					return mysql.getId();
		}
		return "";
	}
}
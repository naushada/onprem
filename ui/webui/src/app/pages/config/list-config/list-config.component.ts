import { Component, OnInit } from '@angular/core';
import { HttpService } from 'src/common/http.service';

@Component({
  selector: 'app-list-config',
  templateUrl: './list-config.component.html',
  styleUrls: ['./list-config.component.scss']
})
export class ListConfigComponent implements OnInit {

  selected:any;
  configs:Array<any> = Array<any>();
  constructor(private http: HttpService) {

  }

  ngOnInit(): void {
    this.onClick();  
  }

  onClick() {
    this.http.gettemplate().subscribe((rsp: string) => {
      this.configs.length = 0;
      let result = JSON.parse(JSON.stringify(rsp));
      if(result["status"] == "success") {
        let payload = JSON.parse(result["response"]);
        this.configs = payload;
      }
    },
    (error) => {

    },
    () => {

    });
  }
}
